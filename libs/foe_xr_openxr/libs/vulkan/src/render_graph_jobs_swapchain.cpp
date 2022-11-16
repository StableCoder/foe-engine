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

foeResultSet foeOpenXrVkImportSwapchainImageRenderJob(
    foeGfxVkRenderGraph renderGraph,
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
    foeGfxVkRenderGraphResource *pXrSwapchainResource,
    foeGfxVkRenderGraphJob *pRenderGraphJob) {
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
        .image = image,
        .view = view,
        .format = format,
        .extent = extent,
    };

    pJobResources->swapchainImageState = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = layout,
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .layerCount = 1,
            },
    };

    foeGfxVkRenderGraphFn freeDataFn = [=]() -> void { delete pJobResources; };

    foeGfxVkRenderGraphResourceCreateInfo resourceCI{
        .sType = FOE_NULL_HANDLE,
        .pName = pResourceName,
        .isMutable = true,
        .pResourceData = &pJobResources->swapchainImage,
    };

    foeGfxVkRenderGraphResource newSwapchainResource;
    foeResultSet result =
        foeGfxVkRenderGraphCreateResource(renderGraph, &resourceCI, &newSwapchainResource);
    if (result.value != FOE_SUCCESS) {
        freeDataFn();
        return result;
    }

    // Add job to graph
    foeGfxVkRenderGraphResourceState resourceState{
        .mode = RENDER_GRAPH_RESOURCE_MODE_READ_WRITE,
        .resource = newSwapchainResource,
        .pIncomingState = (foeGfxVkRenderGraphStructure *)&pJobResources->swapchainImageState,
        .pOutgoingState = nullptr,
    };

    foeGfxVkRenderGraphJobInfo jobInfo{
        .resourceCount = 1,
        .pResources = &resourceState,
        .freeDataFn = freeDataFn,
        .name = pJobName,
        .required = true,
        .pExtraSubmitInfo = &pJobResources->timelineSI,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &semaphore,
        .fence = fence,
    };

    result = foeGfxVkRenderGraphAddJob(renderGraph, &jobInfo, {}, {}, pRenderGraphJob);
    if (result.value != FOE_SUCCESS) {
        // If we couldn't add it to the render graph, delete all heap data now
        freeDataFn();

        return result;
    }

    // Outgoing resources
    *pXrSwapchainResource = newSwapchainResource;

    return to_foeResult(FOE_OPENXR_VK_SUCCESS);
}
