// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/render_graph.hpp>

#include <foe/graphics/vk/session.h>

#include <memory>
#include <queue>

#include "result.h"
#include "vk_result.h"

namespace {

struct RenderGraphJob {
    /// Name of the job for debugging, mapping and logging purposes
    std::string name;
    /// This job needs to be run as part of the render graph's execution, typically as the job is
    /// outputting something, but other reasons exist.
    bool required{false};
    bool processed{false};
    VkQueueFlags queueFlags; // If queue flags is zero, then no preferred family
    void *pExtraSubmitInfo;
    VkFence fence;
    std::vector<VkSemaphore> waitSemaphores;
    std::vector<VkSemaphore> signalSemaphores;
    std::function<foeResultSet(foeGfxSession,
                               foeGfxDelayedCaller,
                               std::vector<VkSemaphore> const &,
                               std::vector<VkSemaphore> const &)>
        customJobFn;
    std::function<foeResultSet(foeGfxSession, foeGfxDelayedCaller, VkCommandBuffer)>
        fillCommandBufferFn;
};

FOE_DEFINE_HANDLE_CASTS(render_graph_job, RenderGraphJob, foeGfxVkRenderGraphJob)

struct RenderGraphRelationship {
    /// Job that provides this resource, determining when it becomes available
    RenderGraphJob *pProvider;
    /// Job that is going to use the resource
    RenderGraphJob *pConsumer;

    /// Resource Data
    foeGfxVkRenderGraphStructure const *pResource;
    /// Whether the consuming job is only going to read the resource
    bool readOnly;
    /// Semaphore used to determine when the resource is available for the consumer
    VkSemaphore semaphore;
};

struct RenderGraph {
    /// Set of calls used to destroy resources after they have been used
    std::vector<std::function<void()>> resourceCleanupCalls;
    /// These are the possible rendering jobs
    std::vector<std::unique_ptr<RenderGraphJob>> jobs;
    /// Set of relationships of resources between jobs
    std::vector<RenderGraphRelationship> relationships;
};

FOE_DEFINE_HANDLE_CASTS(render_graph, RenderGraph, foeGfxVkRenderGraph)

} // namespace

foeGfxVkRenderGraphStructure const *foeGfxVkGraphFindStructure(
    foeGfxVkRenderGraphStructure const *pData, foeGfxVkRenderGraphStructureType sType) {
    while (pData != nullptr) {
        if (pData->sType == sType) {
            return (foeGfxVkRenderGraphStructure *)pData;
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
    for (auto const &freeFn : pRenderGraph->resourceCleanupCalls) {
        freeFn();
    }

    // Delete graph itself
    delete pRenderGraph;
}

foeResultSet foeGfxVkRenderGraphAddJob(
    foeGfxVkRenderGraph renderGraph,
    foeGfxVkRenderGraphJobInfo *pJobInfo,
    std::function<foeResultSet(foeGfxSession,
                               foeGfxDelayedCaller,
                               std::vector<VkSemaphore> const &,
                               std::vector<VkSemaphore> const &)> &&customJob,
    std::function<foeResultSet(foeGfxSession, foeGfxDelayedCaller, VkCommandBuffer)>
        &&fillCommandBufferFn,
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
        .customJobFn = std::move(customJob),
        .fillCommandBufferFn = std::move(fillCommandBufferFn),
    };

    pRenderGraph->jobs.emplace_back(pNewJob.get());

    // Setup relationship in the render graph for each used input resource
    for (uint32_t i = 0; i < pJobInfo->resourceCount; ++i) {
        auto &inRes = pJobInfo->pResourcesIn[i];

        RenderGraphRelationship relationship{
            .pProvider = render_graph_job_from_handle(inRes.provider),
            .pConsumer = pNewJob.get(),
            .pResource = inRes.pResourceData,
            .readOnly = pJobInfo->pResourcesInReadOnly[i],
            .semaphore = VK_NULL_HANDLE,
        };

        pRenderGraph->relationships.emplace_back(relationship);
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
    allSemaphores.reserve(pRenderGraph->relationships.size());

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

    // Go through each job determining the ones we actually need and cull the rest
    while (!toProcess.empty()) {
        auto *pJob = toProcess.front();
        toProcess.pop();

        // Skip the job if we already processed it
        if (pJob->processed)
            continue;

        pJob->processed = true;

        for (auto &relationship : pRenderGraph->relationships) {
            if (relationship.pConsumer == pJob) {
                // If the provider has already been processed, then don't do it again
                if (relationship.pProvider == nullptr)
                    continue;
                if (!relationship.pProvider->processed)
                    toProcess.emplace(relationship.pProvider);

                // This should never happen, as each relationship should be processed only once
                if (relationship.semaphore != VK_NULL_HANDLE)
                    std::abort();

                VkSemaphoreCreateInfo semaphoreCI{
                    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                };

                VkResult vkResult = vkCreateSemaphore(foeGfxVkGetDevice(gfxSession), &semaphoreCI,
                                                      nullptr, &relationship.semaphore);
                if (vkResult != VK_SUCCESS)
                    return vk_to_foeResult(vkResult);

                allSemaphores.emplace_back(relationship.semaphore);
            }
        }
    }

    // EXECUTE
    for (auto const &pJob : pRenderGraph->jobs) {
        // Skip 'culled' jobs
        if (!pJob->processed)
            continue;

        // Figure out the barriers
        std::vector<VkSemaphore> waitSemaphores;
        std::vector<VkSemaphore> signalSemaphores;
        for (auto const &relationship : pRenderGraph->relationships) {
            if (relationship.pConsumer == pJob.get() && relationship.semaphore != VK_NULL_HANDLE)
                waitSemaphores.emplace_back(relationship.semaphore);

            // Only signal if the semaphore exists (some consumers may have been culled)
            if (relationship.pProvider == pJob.get() && relationship.semaphore != VK_NULL_HANDLE)
                signalSemaphores.emplace_back(relationship.semaphore);
        }

        uint32_t const desiredQueueFamily =
            foeGfxVkGetBestQueueFamily(gfxSession, pJob->queueFlags);
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

        if (pJob->fillCommandBufferFn) {
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

            pJob->fillCommandBufferFn(gfxSession, gfxDelayedDestructor, commandBuffer);

            // End Command buffer and submit
            vkResult = vkEndCommandBuffer(commandBuffer);
            if (vkResult != VK_SUCCESS)
                return vk_to_foeResult(vkResult);
        }

        if (!pJob->customJobFn) {
            // Job Submission
            waitSemaphores.insert(waitSemaphores.end(), pJob->waitSemaphores.begin(),
                                  pJob->waitSemaphores.end());
            std::vector<VkPipelineStageFlags> waitMasks(waitSemaphores.size(),
                                                        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

            signalSemaphores.insert(signalSemaphores.end(), pJob->signalSemaphores.begin(),
                                    pJob->signalSemaphores.end());

            VkSubmitInfo submitInfo{
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .pNext = pJob->pExtraSubmitInfo,
                .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
                .pWaitSemaphores = waitSemaphores.data(),
                .pWaitDstStageMask = waitMasks.data(),
                .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
                .pSignalSemaphores = signalSemaphores.data(),
            };
            if (commandBuffer != VK_NULL_HANDLE) {
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &commandBuffer;
            }

            auto queue = foeGfxGetQueue(getFirstQueue(gfxSession));
            vkResult = vkQueueSubmit(queue, 1, &submitInfo, pJob->fence);
            foeGfxReleaseQueue(getFirstQueue(gfxSession), queue);
        } else {
            pJob->customJobFn(gfxSession, gfxDelayedDestructor, waitSemaphores, signalSemaphores);
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