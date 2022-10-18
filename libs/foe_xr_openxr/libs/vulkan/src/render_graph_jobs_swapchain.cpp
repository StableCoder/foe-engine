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
    uint64_t *pWaitValue = new uint64_t;
    *pWaitValue = 1U;

    VkTimelineSemaphoreSubmitInfo *pTimelineSI = new (std::nothrow) VkTimelineSemaphoreSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
        .waitSemaphoreValueCount = 1,
        .pWaitSemaphoreValues = pWaitValue,
    };

    auto *pSwapchain = new (std::nothrow) foeOpenXrRenderGraphSwapchainResource{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_XR_SWAPCHAIN,
        .pNext = nullptr,
        .swapchain = swapchain,
    };

    auto *pImage = new (std::nothrow) foeGfxVkGraphImageResource{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE,
        .pNext = pSwapchain,
        .name = pResourceName,
        .image = image,
        .view = view,
        .format = format,
        .extent = extent,
        .isMutable = true,
    };

    auto *pImageState = new (std::nothrow) foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = layout,
    };

    foeGfxVkRenderGraphFn freeDataFn = [=]() -> void {
        if (pWaitValue)
            delete pWaitValue;
        if (pTimelineSI)
            delete pTimelineSI;
        if (pSwapchain)
            delete pSwapchain;
        if (pImage)
            delete pImage;
        if (pImageState)
            delete pImageState;
    };

    if (pSwapchain == nullptr || pImage == nullptr || pImageState == nullptr) {
        freeDataFn();
        return to_foeResult(FOE_OPENXR_VK_ERROR_OUT_OF_MEMORY);
    }

    // Add job to graph
    foeGfxVkRenderGraphJob renderGraphJob;

    foeGfxVkRenderGraphJobInfo jobInfo{
        .freeDataFn = freeDataFn,
        .name = pJobName,
        .required = true,
        .pExtraSubmitInfo = pTimelineSI,
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
        .pResourceData = reinterpret_cast<foeGfxVkRenderGraphStructure const *>(pImage),
        .pResourceState = reinterpret_cast<foeGfxVkRenderGraphStructure const *>(pImageState),
    };

    return to_foeResult(FOE_OPENXR_VK_SUCCESS);
}
