// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/render_graph/job/export_image.hpp>

#include <foe/graphics/vk/render_graph/resource/image.hpp>
#include <foe/graphics/vk/session.hpp>

#include "../../result.h"
#include "../../vk_result.h"

foeResultSet foeGfxVkExportImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                          std::string_view name,
                                          VkFence fence,
                                          foeGfxVkRenderGraphResource resource,
                                          VkImageLayout requiredLayout,
                                          std::vector<VkSemaphore> signalSemaphores) {
    // Check that this is an image resource
    auto const *pImageData = (foeGfxVkGraphImageResource const *)foeGfxVkGraphFindStructure(
        resource.pResourceData, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE);

    if (pImageData == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_EXPORT_IMAGE_RESOURCE_NOT_IMAGE);

    // Check that this is in the correct/desired state
    auto const *pImageState = (foeGfxVkGraphImageState const *)foeGfxVkGraphFindStructure(
        resource.pResourceState, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE);

    if (pImageState == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_EXPORT_IMAGE_RESOURCE_NO_STATE);
    if (pImageState->layout != requiredLayout)
        std::abort();

    // Proceed with job creation
    auto jobFn = [=](foeGfxSession gfxSession, foeGfxDelayedCaller gfxDelayedDestructor,
                     std::vector<VkSemaphore> const &waitSemaphores,
                     std::vector<VkSemaphore> const &,
                     std::function<void(std::function<void()>)> addCpuFnFn) -> foeResultSet {
        std::vector<VkPipelineStageFlags> waitMasks(waitSemaphores.size(),
                                                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

        VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
            .pWaitSemaphores = waitSemaphores.data(),
            .pWaitDstStageMask = waitMasks.data(),
            .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
            .pSignalSemaphores = signalSemaphores.data(),
        };

        auto queue = foeGfxGetQueue(getFirstQueue(gfxSession));
        VkResult vkResult = vkQueueSubmit(queue, 1, &submitInfo, fence);
        foeGfxReleaseQueue(getFirstQueue(gfxSession), queue);

        return vk_to_foeResult(vkResult);
    };

    // Add job to graph
    bool const readOnly = true;
    foeGfxVkRenderGraphJob renderGraphJob;

    return foeGfxVkRenderGraphAddJob(renderGraph, 1, &resource, &readOnly, nullptr, name, true,
                                     std::move(jobFn), &renderGraphJob);
}