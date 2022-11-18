// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/render_graph.hpp>

#include <foe/graphics/vk/render_graph/resource/image.hpp>
#include <foe/graphics/vk/session.h>

#include <algorithm>
#include <queue>
#include <vector>

#include "result.h"
#include "vk_result.h"

namespace {

struct RenderGraphJob;
struct RenderGraphJobResourceState;

struct RenderGraphResource {
    std::string name;
    bool isMutable;
    void *pResourceData;
    RenderGraphJobResourceState *pFirstJobSeen;
};

FOE_DEFINE_HANDLE_CASTS(render_graph_resource, RenderGraphResource, foeGfxVkRenderGraphResource)

struct RenderGraphJobResourceState {
    RenderGraphJob *pJob;
    RenderGraphResource const *pResource;

    foeGfxVkRenderGraphStructure const *pIncomingState;
    foeGfxVkRenderGraphStructure const *pOutgoingState;
    /// Whether the resource is supposed to be modified in this state
    foeGfxVkRenderGraphResourceMode mode;

    std::vector<RenderGraphJobResourceState *> upstream;
    std::vector<RenderGraphJobResourceState *> downstream;
};

struct RenderGraphJob {
    /// Name of the job for debugging, mapping and logging purposes
    std::string name;
    /// This job needs to be run as part of the render graph's execution, typically as the job is
    /// outputting something, but other reasons exist.
    bool required;
    bool execute;
    VkQueueFlags queueFlags; // If queue flags is zero, then no preferred family
    void *pExtraSubmitInfo;
    VkFence fence;
    std::vector<VkSemaphore> waitSemaphores;
    std::vector<VkSemaphore> signalSemaphores;
    PFN_foeGfxVkRenderGraphCustomSubmit customSubmit;
    PFN_foeGfxVkRenderGraphFillCmdBuffer fillCmdBuf;

    // The state of each resource at the end of the job, before transitions for downstream jobs.
    // This is here because the incomig/outgoing state of resource *may* differ.
    std::vector<RenderGraphJobResourceState> resources;

    std::vector<VkImageMemoryBarrier> incomingBarriers;
    std::vector<VkImageMemoryBarrier> outgoingBarriers;
};

FOE_DEFINE_HANDLE_CASTS(render_graph_job, RenderGraphJob, foeGfxVkRenderGraphJob)

struct RenderGraph {
    // Whether or not the graph has been compiled and is ready to be executed
    bool compiled;
    // Number of jobs after compilation set to be executed
    uint32_t numExecutingJobs;
    /// Set of calls used to destroy resources after they have been used
    std::vector<std::function<void()>> resourceCleanupCalls;
    /// These are the possible rendering jobs
    std::vector<RenderGraphJob *> jobs;

    std::vector<RenderGraphResource *> resources;
};

FOE_DEFINE_HANDLE_CASTS(render_graph, RenderGraph, foeGfxVkRenderGraph)

} // namespace

foeGfxVkRenderGraphStructure const *foeGfxVkGraphFindResourceStructure(
    foeGfxVkRenderGraphResource resource, foeGfxVkRenderGraphStructureType sType) {
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
    for (auto *pJob : pRenderGraph->jobs)
        delete pJob;
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
    foeGfxVkRenderGraphResource *pResource) {
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

char const *foeGfxVkRenderGraphGetResourceName(foeGfxVkRenderGraphResource resource) {
    auto *pResource = render_graph_resource_from_handle(resource);

    return pResource->name.c_str();
}

bool foeGfxVkRenderGraphGetResourceIsMutable(foeGfxVkRenderGraphResource resource) {
    auto *pResource = render_graph_resource_from_handle(resource);

    return pResource->isMutable;
}

foeGfxVkRenderGraphStructure const *foeGfxVkRenderGraphGetResourceData(
    foeGfxVkRenderGraphResource resource) {
    auto *pResource = render_graph_resource_from_handle(resource);

    return (foeGfxVkRenderGraphStructure const *)pResource->pResourceData;
}

foeResultSet foeGfxVkRenderGraphAddJob(foeGfxVkRenderGraph renderGraph,
                                       foeGfxVkRenderGraphJobInfo *pJobInfo,
                                       PFN_foeGfxVkRenderGraphCustomSubmit &&customSubmit,
                                       PFN_foeGfxVkRenderGraphFillCmdBuffer &&fillCmdBuf,
                                       foeGfxVkRenderGraphJob *pJob) {
    auto *pRenderGraph = render_graph_from_handle(renderGraph);

    pRenderGraph->compiled = false;

    // Add job to graph to be run
    RenderGraphJob *pNewJob = new (std::nothrow) RenderGraphJob{
        .name = std::string{pJobInfo->name},
        .required = pJobInfo->required,
        .execute = false,
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
    if (pNewJob == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_OUT_OF_MEMORY);

    pRenderGraph->jobs.emplace_back(pNewJob);

    // Setup relationship in the render graph for each used input resource
    pNewJob->resources.reserve(pJobInfo->resourceCount);
    for (uint32_t i = 0; i < pJobInfo->resourceCount; ++i) {
        auto &resourceState = pJobInfo->pResources[i];
        auto *pResource = render_graph_resource_from_handle(resourceState.resource);

        pNewJob->resources.emplace_back(RenderGraphJobResourceState{
            .pJob = pNewJob,
            .pResource = pResource,
            .pIncomingState = resourceState.pIncomingState,
            .pOutgoingState = resourceState.pOutgoingState,
            .mode = resourceState.mode,
        });

        if (pResource->pFirstJobSeen == nullptr) {
            if (resourceState.upstreamJobCount == 0) {
                pResource->pFirstJobSeen = &pNewJob->resources[i];
            } else {
                // Resource has never been seen/used before, but there's an upstream? What?
                std::abort();
            }
        }

        for (uint32_t j = 0; j < resourceState.upstreamJobCount; ++j) {
            RenderGraphJob *pUpstreamJob =
                render_graph_job_from_handle(resourceState.pUpstreamJobs[j]);

            RenderGraphJobResourceState *pUpstreamResourceState = nullptr;
            for (auto &upstreamResourceState : pUpstreamJob->resources) {
                if (pResource == upstreamResourceState.pResource) {
                    pUpstreamResourceState = &upstreamResourceState;
                    break;
                }
            }

            if (pUpstreamResourceState == nullptr)
                std::abort();

            pUpstreamResourceState->downstream.emplace_back(&pNewJob->resources[i]);

            pNewJob->resources[i].upstream.emplace_back(pUpstreamResourceState);
        }
    }

    // Add any calls for deleting resources at cleanup
    if (pJobInfo->freeDataFn)
        pRenderGraph->resourceCleanupCalls.emplace_back(std::move(pJobInfo->freeDataFn));

    *pJob = render_graph_job_to_handle(pNewJob);

    return to_foeResult(FOE_GRAPHICS_VK_SUCCESS);
}

bool foeGfxVkRenderGraphJobToExecute(foeGfxVkRenderGraphJob job) {
    auto *pJob = render_graph_job_from_handle(job);

    return pJob->execute;
}

foeResultSet foeGfxVkRenderGraphCompile(foeGfxVkRenderGraph renderGraph) {
    auto *pRenderGraph = render_graph_from_handle(renderGraph);

    if (pRenderGraph->jobs.empty()) {
        pRenderGraph->compiled = true;
        return to_foeResult(FOE_GRAPHICS_VK_NO_JOBS_TO_COMPILE);
    }

    // The list of jobs currently set to be processed moving forward
    std::queue<RenderGraphJob *> toProcess;

    // Find all jobs that are required to run and add them to be processed
    for (auto *pJob : pRenderGraph->jobs) {
        if (pJob->required) {
            toProcess.push(pJob);
        }
    }

    // Going downstream to upstream
    uint32_t numNewJobs = 0;
    while (!toProcess.empty()) {
        auto *pJob = toProcess.front();
        toProcess.pop();

        // Skip the job if we already processed it
        if (pJob->execute)
            continue;

        ++numNewJobs;
        pJob->execute = true;

        for (auto const &resource : pJob->resources) {
            // If the upstream job has not yet been processed, add it
            for (auto *pUpstreamResourceState : resource.upstream) {
                if (!pUpstreamResourceState->pJob->execute) {
                    toProcess.emplace(pUpstreamResourceState->pJob);
                }
            }
        }
    }

    for (auto *pResource : pRenderGraph->resources) {
        if (pResource->pFirstJobSeen == nullptr)
            continue;

        std::vector<RenderGraphJobResourceState *> currentUseWave{pResource->pFirstJobSeen};
        std::vector<RenderGraphJobResourceState *> nextUseWave;

        while (true) {
            // Get the next wave of active uses of this resource
            for (auto *pResourceState : currentUseWave) {
                for (auto *pDownstreamState : pResourceState->downstream) {
                    if (pDownstreamState->pJob->execute)
                        nextUseWave.emplace_back(pDownstreamState);
                }
            }

            if (nextUseWave.empty())
                // Found no more iterations of the resource or jobs
                break;

            // Sort and remove duplicates
            std::sort(nextUseWave.begin(), nextUseWave.end());
            nextUseWave.erase(std::unique(nextUseWave.begin(), nextUseWave.end()),
                              nextUseWave.end());

            foeGfxVkRenderGraphStructure const *pUpstreamReference =
                currentUseWave[0]->pOutgoingState;

            if (currentUseWave.size() > 1) {
                for (size_t i = 1; i < currentUseWave.size(); ++i) {
                    foeGfxVkRenderGraphStructure const *pCompare =
                        currentUseWave[i]->pOutgoingState;

                    if (pUpstreamReference->sType != pCompare->sType)
                        std::abort();

                    // Only images
                    if (((foeGfxVkGraphImageState const *)pUpstreamReference)->layout !=
                        ((foeGfxVkGraphImageState const *)pCompare)->layout)
                        std::abort();
                }
            }

            foeGfxVkRenderGraphStructure const *pDownstreamReference =
                nextUseWave[0]->pIncomingState;

            if (nextUseWave.size() > 1) {
                for (size_t i = 1; i < nextUseWave.size(); ++i) {
                    foeGfxVkRenderGraphStructure const *pCompare = nextUseWave[i]->pIncomingState;

                    if (pDownstreamReference->sType != pCompare->sType)
                        std::abort();

                    // Only images
                    if (((foeGfxVkGraphImageState const *)pDownstreamReference)->layout !=
                        ((foeGfxVkGraphImageState const *)pCompare)->layout)
                        std::abort();
                }
            }

            if (((foeGfxVkGraphImageState const *)pUpstreamReference)->layout !=
                ((foeGfxVkGraphImageState const *)pDownstreamReference)->layout) {
                // Create transition state
                VkImageMemoryBarrier imageBarrier = {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .srcAccessMask = foeGfxVkDetermineAccessFlags(
                        ((foeGfxVkGraphImageState const *)pUpstreamReference)->layout),
                    .dstAccessMask = foeGfxVkDetermineAccessFlags(
                        ((foeGfxVkGraphImageState const *)pDownstreamReference)->layout),
                    .oldLayout = ((foeGfxVkGraphImageState const *)pUpstreamReference)->layout,
                    .newLayout = ((foeGfxVkGraphImageState const *)pDownstreamReference)->layout,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image = ((foeGfxVkGraphImageResource const *)pResource->pResourceData)->image,
                    .subresourceRange =
                        ((foeGfxVkGraphImageState const *)pUpstreamReference)->subresourceRange,
                };

                if (currentUseWave[0]->mode == RENDER_GRAPH_RESOURCE_MODE_READ_WRITE) {
                    // Should only be max of one read/write per wave
                    currentUseWave[0]->pJob->outgoingBarriers.emplace_back(imageBarrier);
                } else if (nextUseWave[0]->mode == RENDER_GRAPH_RESOURCE_MODE_READ_WRITE) {
                    nextUseWave[0]->pJob->incomingBarriers.emplace_back(imageBarrier);
                } else {
                    // Need to insert a synchronization job between the two 'real' jobs
                    std::abort();
                }
            }

            // Do a swap and clear the next wave, save some de/alloc calls
            std::swap(currentUseWave, nextUseWave);
            nextUseWave.clear();
        }
    }

    pRenderGraph->compiled = true;
    pRenderGraph->numExecutingJobs += numNewJobs;
    return to_foeResult((pRenderGraph->numExecutingJobs != 0) ? FOE_GRAPHICS_VK_SUCCESS
                                                              : FOE_GRAPHICS_VK_NO_JOBS_TO_EXECUTE);
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

foeResultSet foeGfxVkRenderGraphExecute(foeGfxVkRenderGraph renderGraph,
                                        foeGfxSession gfxSession,
                                        foeGfxDelayedCaller gfxDelayedDestructor) {
    auto *pRenderGraph = render_graph_from_handle(renderGraph);

    if (!pRenderGraph->compiled)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_NOT_COMPILED);
    if (pRenderGraph->numExecutingJobs == 0)
        return to_foeResult(FOE_GRAPHICS_VK_NO_JOBS_TO_EXECUTE);

    VkResult vkResult = VK_SUCCESS;
    uint32_t const numQueueFamilies = foeGfxVkGetNumQueueFamilies(gfxSession);

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

    // Going upstream to downstream
    bool anyJobExecuted = false;
    for (auto const &pJob : pRenderGraph->jobs) {
        // Skip 'culled' jobs
        if (!pJob->execute)
            continue;
        anyJobExecuted = true;

        // Figure out the barriers
        std::vector<VkSemaphore> &waitSemaphores = pJob->waitSemaphores;
        std::vector<VkSemaphore> &signalSemaphores = pJob->signalSemaphores;

        for (auto &resource : pJob->resources) {
            for (auto &downstreamResource : resource.downstream) {
                RenderGraphJob *const pDownstreamJob = downstreamResource->pJob;
                if (!pDownstreamJob->execute)
                    // Culled job
                    continue;

                VkSemaphoreCreateInfo semaphoreCI{
                    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                };

                VkSemaphore semaphore;
                VkResult vkResult = vkCreateSemaphore(foeGfxVkGetDevice(gfxSession), &semaphoreCI,
                                                      nullptr, &semaphore);
                if (vkResult != VK_SUCCESS)
                    return vk_to_foeResult(vkResult);

                allSemaphores.emplace_back(semaphore);
                signalSemaphores.emplace_back(semaphore);
                pDownstreamJob->waitSemaphores.emplace_back(semaphore);
            }
        }

        uint32_t const desiredQueueFamily =
            foeGfxVkGetBestQueueFamily(gfxSession, pJob->queueFlags);
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

        if (pJob->fillCmdBuf || !pJob->incomingBarriers.empty() ||
            !pJob->outgoingBarriers.empty()) {
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

            if (!pJob->incomingBarriers.empty())
                vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr,
                                     pJob->incomingBarriers.size(), pJob->incomingBarriers.data());

            if (pJob->fillCmdBuf)
                pJob->fillCmdBuf(gfxSession, gfxDelayedDestructor, commandBuffer);

            if (!pJob->outgoingBarriers.empty())
                vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr,
                                     pJob->outgoingBarriers.size(), pJob->outgoingBarriers.data());

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

    if (!anyJobExecuted)
        return to_foeResult(FOE_GRAPHICS_VK_NO_JOBS_TO_EXECUTE);

    // Cleanup graph semaphores after execution
    std::vector<VkSemaphore> const *pAllSemaphores =
        new std::vector<VkSemaphore>{std::move(allSemaphores)};

    foeGfxAddDefaultDelayedCall(gfxDelayedDestructor,
                                (PFN_foeGfxDelayedCall)destroy_RenderGraphSemaphores,
                                (void *)pAllSemaphores);

    return to_foeResult(FOE_GRAPHICS_VK_SUCCESS);
}