// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/xr/openxr/vk/render_graph_jobs_swapchain.hpp>

#include <foe/graphics/vk/render_graph/resource/image.hpp>
#include <foe/graphics/vk/session.h>

#include "log.hpp"
#include "result.h"
#include "vk_result.h"
#include "xr_result.h"

struct foeOpenXrRenderGraphSwapchainResource {
    foeGfxVkRenderGraphStructureType sType;
    void *pNext;
    XrSwapchain swapchain;
};

foeResultSet foeOpenXrVkImportSwapchainImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                                      char const *pJobName,
                                                      VkFence fence,
                                                      char const *pResourceName,
                                                      VkSemaphore semaphore,
                                                      XrSwapchain swapchain,
                                                      VkImage image,
                                                      VkImageView view,
                                                      VkFormat format,
                                                      VkExtent2D extent,
                                                      VkImageLayout layout,
                                                      foeGfxVkRenderGraphResource *pResourcesOut) {
    // Resource management
    struct ImportXrSwapchainImageJobResources {
        uint64_t waitValue;
        VkTimelineSemaphoreSubmitInfo timelineSI;
        foeOpenXrRenderGraphSwapchainResource swapchainImageResource;
        foeGfxVkGraphImageResource swapchainImage;
        foeGfxVkGraphImageState swapchainImageState;
    };

    ImportXrSwapchainImageJobResources *pJobResources =
        new (std::nothrow) ImportXrSwapchainImageJobResources;
    if (pJobResources == nullptr)
        return to_foeResult(FOE_OPENXR_VK_ERROR_OUT_OF_MEMORY);

    pJobResources->waitValue = 1U;

    pJobResources->timelineSI = VkTimelineSemaphoreSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
        .waitSemaphoreValueCount = 1,
        .pWaitSemaphoreValues = &pJobResources->waitValue,
    };

    pJobResources->swapchainImageResource = foeOpenXrRenderGraphSwapchainResource{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_XR_SWAPCHAIN,
        .pNext = nullptr,
        .swapchain = swapchain,
    };

    pJobResources->swapchainImage = foeGfxVkGraphImageResource{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE,
        .pNext = &pJobResources->swapchainImageResource,
        .name = pResourceName,
        .image = image,
        .view = view,
        .format = format,
        .extent = extent,
        .isMutable = true,
    };

    pJobResources->swapchainImageState = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = layout,
    };

    foeGfxVkRenderGraphFn freeDataFn = [=]() -> void { delete pJobResources; };

    // Add job to graph
    foeGfxVkRenderGraphJob renderGraphJob;

    foeGfxVkRenderGraphJobInfo jobInfo{
        .freeDataFn = freeDataFn,
        .name = pJobName,
        .required = true,
        .pExtraSubmitInfo = &pJobResources->timelineSI,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &semaphore,
        .fence = fence,
    };

    foeResultSet result = foeGfxVkRenderGraphAddJob(renderGraph, &jobInfo, {}, {}, &renderGraphJob);
    if (result.value != FOE_SUCCESS) {
        // If we couldn't add it to the render graph, delete all heap data now
        freeDataFn();

        return result;
    }

    // Outgoing resources
    *pResourcesOut = foeGfxVkRenderGraphResource{
        .provider = renderGraphJob,
        .pResourceData =
            reinterpret_cast<foeGfxVkRenderGraphStructure const *>(&pJobResources->swapchainImage),
        .pResourceState = reinterpret_cast<foeGfxVkRenderGraphStructure const *>(
            &pJobResources->swapchainImageState),
    };

    return to_foeResult(FOE_OPENXR_VK_SUCCESS);
}
