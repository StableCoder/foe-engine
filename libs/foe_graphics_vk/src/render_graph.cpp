/*
    Copyright (C) 2022 George Cave.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <foe/graphics/vk/render_graph.hpp>

#include <foe/graphics/vk/session.hpp>
#include <vk_error_code.hpp>

#include <memory>
#include <queue>

#include "error_code.hpp"

namespace {

struct RenderGraphJob {
    /// Name of the job for debugging, mapping and logging purposes
    std::string name;
    /// This job needs to be run as part of the render graph's execution, typically as the job is
    /// outputting something, but other reasons exist.
    bool required{false};
    bool processed{false};
    std::function<std::error_code(foeGfxSession,
                                  foeGfxDelayedDestructor,
                                  std::vector<VkSemaphore> const &,
                                  std::vector<VkSemaphore> const &,
                                  std::function<void(std::function<void()>)>)>
        jobFn;
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
    std::vector<DeleteResourceDataCall> resourceCleanupCalls;
    /// These are the possible rendering jobs
    std::vector<std::unique_ptr<RenderGraphJob>> jobs;
    /// These are the CPU jobs that GPU jobs will be waiting on for signals
    std::vector<std::function<void()>> cpuJobs;
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

auto foeGfxVkCreateRenderGraph(foeGfxVkRenderGraph *pRenderGraph) -> std::error_code {
    RenderGraph *pNewRenderGraph = new RenderGraph;
    *pNewRenderGraph = {};

    *pRenderGraph = render_graph_to_handle(pNewRenderGraph);

    return FOE_GRAPHICS_VK_SUCCESS;
}

void foeGfxVkDestroyRenderGraph(foeGfxVkRenderGraph renderGraph) {
    auto *pRenderGraph = render_graph_from_handle(renderGraph);

    // Delete RenderGraph content
    for (auto const &callData : pRenderGraph->resourceCleanupCalls) {
        callData.deleteFn(callData.pResource);
    }

    // Delete graph itself
    delete pRenderGraph;
}

auto foeGfxVkRenderGraphAddJob(
    foeGfxVkRenderGraph renderGraph,
    uint32_t resourcesCount,
    foeGfxVkRenderGraphResource const *pResourcesIn,
    bool const *pResourcesInReadOnly,
    uint32_t deleteResourceCallsCount,
    DeleteResourceDataCall *pDeleteResourceCalls,
    std::string_view name,
    bool required,
    std::function<std::error_code(foeGfxSession,
                                  foeGfxDelayedDestructor,
                                  std::vector<VkSemaphore> const &,
                                  std::vector<VkSemaphore> const &,
                                  std::function<void(std::function<void()>)>)> &&jobFn,
    foeGfxVkRenderGraphJob *pJob) -> std::error_code {
    auto *pRenderGraph = render_graph_from_handle(renderGraph);

    // Add job to graph to be run
    std::unique_ptr<RenderGraphJob> pNewJob{new RenderGraphJob};

    *pNewJob = RenderGraphJob{
        .name = std::string{name},
        .required = required,
        .processed = false,
        .jobFn = std::move(jobFn),
    };

    pRenderGraph->jobs.emplace_back(pNewJob.get());

    // Setup relationship in the render graph for each used input resource
    for (uint32_t i = 0; i < resourcesCount; ++i) {
        auto &inRes = pResourcesIn[i];

        RenderGraphRelationship relationship{
            .pProvider = render_graph_job_from_handle(inRes.provider),
            .pConsumer = pNewJob.get(),
            .pResource = inRes.pResourceData,
            .readOnly = pResourcesInReadOnly[i],
            .semaphore = VK_NULL_HANDLE,
        };

        pRenderGraph->relationships.emplace_back(relationship);
    }

    // Add any calls for deleting resources at cleanup
    for (uint32_t i = 0; i < deleteResourceCallsCount; ++i) {
        pRenderGraph->resourceCleanupCalls.emplace_back(pDeleteResourceCalls[i]);
    }

    *pJob = render_graph_job_to_handle(pNewJob.release());

    return FOE_GRAPHICS_VK_SUCCESS;
}

auto foeGfxVkExecuteRenderGraph(foeGfxVkRenderGraph renderGraph,
                                foeGfxSession gfxSession,
                                foeGfxDelayedDestructor gfxDelayedDestructor) -> std::error_code {
    auto *pRenderGraph = render_graph_from_handle(renderGraph);
    std::error_code errC;

    // COMPILE

    // The list of jobs currently set to be processed moving forward
    std::queue<RenderGraphJob *> toProcess;
    // The list of semaphores created as part of the compile to be later destroyed
    std::vector<VkSemaphore> allSemaphores;
    allSemaphores.reserve(pRenderGraph->relationships.size());

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

                errC = vkCreateSemaphore(foeGfxVkGetDevice(gfxSession), &semaphoreCI, nullptr,
                                         &relationship.semaphore);
                if (errC)
                    return errC;

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

        pJob->jobFn(gfxSession, gfxDelayedDestructor, waitSemaphores, signalSemaphores,
                    [&](std::function<void()> fn) { pRenderGraph->cpuJobs.emplace_back(fn); });
    }

    // Cleanup graph semaphores after execution
    foeGfxAddDelayedDestructionCall(gfxDelayedDestructor, [=](foeGfxSession session) {
        for (auto it : allSemaphores) {
            vkDestroySemaphore(foeGfxVkGetDevice(session), it, nullptr);
        }
    });

    return FOE_GRAPHICS_VK_SUCCESS;
}

void foeGfxVkExecuteRenderGraphCpuJobs(foeGfxVkRenderGraph renderGraph) {
    auto *pRenderGraph = render_graph_from_handle(renderGraph);

    for (auto const &job : pRenderGraph->cpuJobs) {
        job();
    }
}