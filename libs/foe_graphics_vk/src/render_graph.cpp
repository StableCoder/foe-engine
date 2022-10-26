// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/render_graph.hpp>

#include <foe/graphics/vk/session.h>

#include <memory>
#include <queue>
#include <vector>

#include "result.h"
#include "vk_result.h"

namespace {

struct RenderGraphJob;

struct RenderGraphResource {
    std::string name;
    bool isMutable;
    void *pResourceData;
};

FOE_DEFINE_HANDLE_CASTS(render_graph_resource,
                        RenderGraphResource,
                        foeGfxVkRenderGraphResourceHandle)

struct RenderGraphJobResourceState {
    /// Resource Data
    RenderGraphResource const *pResource;
    // Desired Resource State
    foeGfxVkRenderGraphStructure const *pResourceState;
    /// Whether the resource is supposed to be modified in this state
    bool readOnly;
};

struct RenderGraphDownstreamJobData {
    RenderGraphJob *pJob;
    /// Resource Data
    RenderGraphResource const *pResource;
    void const *pResourceState;
    /// Whether the downstream job is only going to read the resource
    bool readOnly;
};

struct RenderGraphJob {
    /// Name of the job for debugging, mapping and logging purposes
    std::string name;
    /// This job needs to be run as part of the render graph's execution, typically as the job is
    /// outputting something, but other reasons exist.
    bool required;
    bool processed;
    VkQueueFlags queueFlags; // If queue flags is zero, then no preferred family
    void *pExtraSubmitInfo;
    VkFence fence;
    std::vector<VkSemaphore> waitSemaphores;
    std::vector<VkSemaphore> signalSemaphores;
    PFN_foeGfxVkRenderGraphCustomSubmit customSubmit;
    PFN_foeGfxVkRenderGraphFillCmdBuffer fillCmdBuf;

    std::vector<RenderGraphJob *> upstreamJobs;
    std::vector<RenderGraphDownstreamJobData> downstreamJobs;
};

FOE_DEFINE_HANDLE_CASTS(render_graph_job, RenderGraphJob, foeGfxVkRenderGraphJob)

struct RenderGraph {
    /// Set of calls used to destroy resources after they have been used
    std::vector<std::function<void()>> resourceCleanupCalls;
    /// These are the possible rendering jobs
    std::vector<std::unique_ptr<RenderGraphJob>> jobs;

    std::vector<RenderGraphResource *> resources;
};

FOE_DEFINE_HANDLE_CASTS(render_graph, RenderGraph, foeGfxVkRenderGraph)

} // namespace

foeGfxVkRenderGraphStructure const *foeGfxVkGraphFindResourceStructure(
    foeGfxVkRenderGraphResourceHandle resource, foeGfxVkRenderGraphStructureType sType) {
    auto *pResource = render_graph_resource_from_handle(resource);
    foeGfxVkRenderGraphStructure const *pData =
        (foeGfxVkRenderGraphStructure const *)pResource->pResourceData;

    while (pData != nullptr) {
        if (pData->sType == sType) {
            return (foeGfxVkRenderGraphStructure const *)pData;
        }

        pData = (foeGfxVkRenderGraphStructure const *)pData->pNext;
    }

    return nullptr;
}

foeGfxVkRenderGraphStructure const *foeGfxVkGraphFindStructure(
    foeGfxVkRenderGraphStructure const *pData, foeGfxVkRenderGraphStructureType sType) {
    while (pData != nullptr) {
        if (pData->sType == sType) {
            return (foeGfxVkRenderGraphStructure const *)pData;
        }

        pData = (foeGfxVkRenderGraphStructure const *)pData->pNext;
    }

    return nullptr;
}

foeResultSet foeGfxVkCreateRenderGraph(foeGfxVkRenderGraph *pRenderGraph) {
    RenderGraph *pNewRenderGraph = new RenderGraph;
    *pNewRenderGraph = {};

    *pRenderGraph = render_graph_to_handle(pNewRenderGraph);

    return to_foeResult(FOE_GRAPHICS_VK_SUCCESS);
}

void foeGfxVkDestroyRenderGraph(foeGfxVkRenderGraph renderGraph) {
    auto *pRenderGraph = render_graph_from_handle(renderGraph);

    // Delete RenderGraph content
    for (auto *pResource : pRenderGraph->resources)
        delete pResource;
    for (auto const &freeFn : pRenderGraph->resourceCleanupCalls) {
        freeFn();
    }

    // Delete graph itself
    delete pRenderGraph;
}

foeResultSet foeGfxVkRenderGraphCreateResource(
    foeGfxVkRenderGraph renderGraph,
    foeGfxVkRenderGraphResourceCreateInfo *pResoureCreateInfo,
    foeGfxVkRenderGraphResourceHandle *pResource) {
    auto *pRenderGraph = render_graph_from_handle(renderGraph);

    RenderGraphResource *pNewResource = new (std::nothrow) RenderGraphResource{
        .name = pResoureCreateInfo->pName,
        .isMutable = pResoureCreateInfo->isMutable,
        .pResourceData = pResoureCreateInfo->pResourceData,
    };
    if (pNewResource == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_OUT_OF_MEMORY);

    pRenderGraph->resources.emplace_back(pNewResource);

    *pResource = render_graph_resource_to_handle(pNewResource);

    return to_foeResult(FOE_GRAPHICS_VK_SUCCESS);
}

char const *foeGfxVkRenderGraphGetResourceName(foeGfxVkRenderGraphResourceHandle resource) {
    auto *pResource = render_graph_resource_from_handle(resource);

    return pResource->name.c_str();
}

bool foeGfxVkRenderGraphGetResourceIsMutable(foeGfxVkRenderGraphResourceHandle resource) {
    auto *pResource = render_graph_resource_from_handle(resource);

    return pResource->isMutable;
}

foeGfxVkRenderGraphStructure const *foeGfxVkRenderGraphGetResourceData(
    foeGfxVkRenderGraphResourceHandle resource) {
    auto *pResource = render_graph_resource_from_handle(resource);

    return (foeGfxVkRenderGraphStructure const *)pResource->pResourceData;
}

foeResultSet foeGfxVkRenderGraphAddJob(foeGfxVkRenderGraph renderGraph,
                                       foeGfxVkRenderGraphJobInfo *pJobInfo,
                                       PFN_foeGfxVkRenderGraphCustomSubmit &&customSubmit,
                                       PFN_foeGfxVkRenderGraphFillCmdBuffer &&fillCmdBuf,
                                       foeGfxVkRenderGraphJob *pJob) {
    auto *pRenderGraph = render_graph_from_handle(renderGraph);

    // Add job to graph to be run
    std::unique_ptr<RenderGraphJob> pNewJob{new RenderGraphJob};

    *pNewJob = RenderGraphJob{
        .name = std::string{pJobInfo->name},
        .required = pJobInfo->required,
        .processed = false,
        .queueFlags = pJobInfo->queueFlags,
        .pExtraSubmitInfo = pJobInfo->pExtraSubmitInfo,
        .fence = pJobInfo->fence,
        .waitSemaphores = {pJobInfo->pWaitSemaphores,
                           pJobInfo->pWaitSemaphores + pJobInfo->waitSemaphoreCount},
        .signalSemaphores = {pJobInfo->pSignalSemaphores,
                             pJobInfo->pSignalSemaphores + pJobInfo->signalSemaphoreCount},
        .customSubmit = std::move(customSubmit),
        .fillCmdBuf = std::move(fillCmdBuf),
    };

    pRenderGraph->jobs.emplace_back(pNewJob.get());

    // Setup relationship in the render graph for each used input resource
    for (uint32_t i = 0; i < pJobInfo->resourceCount; ++i) {
        auto &inRes = pJobInfo->pResourcesIn[i];

        RenderGraphJob *pUpstreamJob = render_graph_job_from_handle(inRes.provider);

        pNewJob->upstreamJobs.emplace_back(pUpstreamJob);
        pUpstreamJob->downstreamJobs.emplace_back(RenderGraphDownstreamJobData{
            .pJob = pNewJob.get(),
            .pResource = render_graph_resource_from_handle(inRes.resource),
            .readOnly = pJobInfo->pResourcesInReadOnly[i],
        });
    }

    // Add any calls for deleting resources at cleanup
    if (pJobInfo->freeDataFn)
        pRenderGraph->resourceCleanupCalls.emplace_back(std::move(pJobInfo->freeDataFn));

    *pJob = render_graph_job_to_handle(pNewJob.release());

    return to_foeResult(FOE_GRAPHICS_VK_SUCCESS);
}

namespace {

void cleanupOldCommandPool(VkCommandPool commandPool, foeGfxSession session) {
    vkDestroyCommandPool(foeGfxVkGetDevice(session), commandPool, nullptr);
}

void destroy_RenderGraphSemaphores(std::vector<VkSemaphore> const *pAllSemaphores,
                                   foeGfxSession session) {
    for (auto it : *pAllSemaphores) {
        vkDestroySemaphore(foeGfxVkGetDevice(session), it, nullptr);
    }

    delete pAllSemaphores;
}

} // namespace

foeResultSet foeGfxVkExecuteRenderGraph(foeGfxVkRenderGraph renderGraph,
                                        foeGfxSession gfxSession,
                                        foeGfxDelayedCaller gfxDelayedDestructor) {
    auto *pRenderGraph = render_graph_from_handle(renderGraph);

    VkResult vkResult = VK_SUCCESS;
    uint32_t const numQueueFamilies = foeGfxVkGetNumQueueFamilies(gfxSession);

    // COMPILE

    // The list of jobs currently set to be processed moving forward
    std::queue<RenderGraphJob *> toProcess;
    // The list of semaphores created as part of the compile to be later destroyed
    std::vector<VkSemaphore> allSemaphores;

    // Command Pools
    std::vector<VkCommandPool> commandPools;

    for (uint32_t i = 0; i < numQueueFamilies; ++i) {
        VkCommandPoolCreateInfo poolCI{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = 0,
        };
        VkCommandPool commandPool;
        vkResult =
            vkCreateCommandPool(foeGfxVkGetDevice(gfxSession), &poolCI, nullptr, &commandPool);
        if (vkResult != VK_SUCCESS)
            return vk_to_foeResult(vkResult);

        foeGfxAddDefaultDelayedCall(gfxDelayedDestructor,
                                    (PFN_foeGfxDelayedCall)cleanupOldCommandPool,
                                    (void *)commandPool);

        commandPools.emplace_back(commandPool);
    }

    // Find all jobs that are required to run and add them to be processed
    for (auto const &pJob : pRenderGraph->jobs) {
        if (pJob->required) {
            toProcess.push(pJob.get());
        }
    }

    // VALIDATION and CULLING
    // Going downstream to upstream
    while (!toProcess.empty()) {
        auto *pJob = toProcess.front();
        toProcess.pop();

        // Skip the job if we already processed it
        if (pJob->processed)
            continue;

        pJob->processed = true;

        for (auto *pUpstreamJob : pJob->upstreamJobs) {
            // If the upstream job has not yet been processed, add it
            if (!pUpstreamJob->processed)
                toProcess.emplace(pUpstreamJob);
        }
    }

    // EXECUTION
    // Going upstream to downstream
    for (auto const &pJob : pRenderGraph->jobs) {
        // Skip 'culled' jobs
        if (!pJob->processed)
            continue;

        // Figure out the barriers
        std::vector<VkSemaphore> &waitSemaphores = pJob->waitSemaphores;
        std::vector<VkSemaphore> &signalSemaphores = pJob->signalSemaphores;

        for (RenderGraphDownstreamJobData &downstreamJob : pJob->downstreamJobs) {
            if (!downstreamJob.pJob->processed)
                // Culled job
                continue;

            VkSemaphoreCreateInfo semaphoreCI{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            };

            VkSemaphore semaphore;
            VkResult vkResult =
                vkCreateSemaphore(foeGfxVkGetDevice(gfxSession), &semaphoreCI, nullptr, &semaphore);
            if (vkResult != VK_SUCCESS)
                return vk_to_foeResult(vkResult);

            allSemaphores.emplace_back(semaphore);
            signalSemaphores.emplace_back(semaphore);
            downstreamJob.pJob->waitSemaphores.emplace_back(semaphore);
        }

        uint32_t const desiredQueueFamily =
            foeGfxVkGetBestQueueFamily(gfxSession, pJob->queueFlags);
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

        if (pJob->fillCmdBuf) {
            // Create CommandBuffer
            VkCommandBufferAllocateInfo commandBufferAI{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = commandPools[desiredQueueFamily],
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
            };

            vkResult = vkAllocateCommandBuffers(foeGfxVkGetDevice(gfxSession), &commandBufferAI,
                                                &commandBuffer);
            if (vkResult != VK_SUCCESS)
                return vk_to_foeResult(vkResult);
            // Start CommandBuffer
            VkCommandBufferBeginInfo commandBufferBI{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            };
            vkResult = vkBeginCommandBuffer(commandBuffer, &commandBufferBI);
            if (vkResult != VK_SUCCESS)
                return vk_to_foeResult(vkResult);

            pJob->fillCmdBuf(gfxSession, gfxDelayedDestructor, commandBuffer);

            // End Command buffer and submit
            vkResult = vkEndCommandBuffer(commandBuffer);
            if (vkResult != VK_SUCCESS)
                return vk_to_foeResult(vkResult);
        }

        if (!pJob->customSubmit) {
            // Job Submission
            std::vector<VkPipelineStageFlags> waitMasks(waitSemaphores.size(),
                                                        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

            VkSubmitInfo submitInfo{
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .pNext = pJob->pExtraSubmitInfo,
                .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
                .pWaitSemaphores = waitSemaphores.data(),
                .pWaitDstStageMask = waitMasks.data(),
                .commandBufferCount = (commandBuffer != VK_NULL_HANDLE) ? 1U : 0U,
                .pCommandBuffers = &commandBuffer,
                .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
                .pSignalSemaphores = signalSemaphores.data(),
            };

            auto queue = foeGfxGetQueue(getFirstQueue(gfxSession));
            vkResult = vkQueueSubmit(queue, 1, &submitInfo, pJob->fence);
            foeGfxReleaseQueue(getFirstQueue(gfxSession), queue);
        } else {
            pJob->customSubmit(gfxSession, gfxDelayedDestructor, waitSemaphores.size(),
                               waitSemaphores.data(), (commandBuffer != VK_NULL_HANDLE) ? 1 : 0,
                               &commandBuffer, signalSemaphores.size(), signalSemaphores.data(),
                               pJob->fence);
        }
    }

    // Cleanup graph semaphores after execution
    std::vector<VkSemaphore> const *pAllSemaphores =
        new std::vector<VkSemaphore>{std::move(allSemaphores)};

    foeGfxAddDefaultDelayedCall(gfxDelayedDestructor,
                                (PFN_foeGfxDelayedCall)destroy_RenderGraphSemaphores,
                                (void *)pAllSemaphores);

    return to_foeResult(FOE_GRAPHICS_VK_SUCCESS);
}