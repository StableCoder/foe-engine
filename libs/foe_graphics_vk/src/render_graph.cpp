// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/render_graph.hpp>

#include <foe/graphics/vk/session.hpp>

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
    std::function<foeResultSet(foeGfxSession,
                               foeGfxDelayedCaller,
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
    std::vector<std::function<void()>> resourceCleanupCalls;
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
    uint32_t resourcesCount,
    foeGfxVkRenderGraphResource const *pResourcesIn,
    bool const *pResourcesInReadOnly,
    foeGfxVkRenderGraphFn freeDataFn,
    std::string_view name,
    bool required,
    std::function<foeResultSet(foeGfxSession,
                               foeGfxDelayedCaller,
                               std::vector<VkSemaphore> const &,
                               std::vector<VkSemaphore> const &,
                               std::function<void(std::function<void()>)>)> &&jobFn,
    foeGfxVkRenderGraphJob *pJob) {
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
    if (freeDataFn)
        pRenderGraph->resourceCleanupCalls.emplace_back(std::move(freeDataFn));

    *pJob = render_graph_job_to_handle(pNewJob.release());

    return to_foeResult(FOE_GRAPHICS_VK_SUCCESS);
}

namespace {

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

        pJob->jobFn(gfxSession, gfxDelayedDestructor, waitSemaphores, signalSemaphores,
                    [&](std::function<void()> fn) { pRenderGraph->cpuJobs.emplace_back(fn); });
    }

    // Cleanup graph semaphores after execution
    std::vector<VkSemaphore> const *pAllSemaphores =
        new std::vector<VkSemaphore>{std::move(allSemaphores)};

    foeGfxAddDefaultDelayedCall(gfxDelayedDestructor,
                                (PFN_foeGfxDelayedCall)destroy_RenderGraphSemaphores,
                                (void *)pAllSemaphores);

    return to_foeResult(FOE_GRAPHICS_VK_SUCCESS);
}

void foeGfxVkExecuteRenderGraphCpuJobs(foeGfxVkRenderGraph renderGraph) {
    auto *pRenderGraph = render_graph_from_handle(renderGraph);

    for (auto const &job : pRenderGraph->cpuJobs) {
        job();
    }
}