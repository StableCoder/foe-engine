// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/render_graph/job/import_image.hpp>

#include <foe/graphics/vk/image.h>
#include <foe/graphics/vk/render_graph/resource/image.hpp>
#include <foe/graphics/vk/session.h>

#include "../../result.h"
#include "../../vk_result.h"

foeResultSet foeGfxVkImportImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                          char const *pJobName,
                                          VkFence fence,
                                          char const *pResourceName,
                                          VkImage image,
                                          VkImageView view,
                                          VkFormat format,
                                          VkExtent2D extent,
                                          VkImageLayout layout,
                                          bool isMutable,
                                          std::vector<VkSemaphore> waitSemaphores,
                                          foeGfxVkRenderGraphResource *pImportedImageResource,
                                          foeGfxVkRenderGraphJob *pRenderGraphJob) {
    foeResultSet result;

    // Resource management
    struct ImportImageJobResources {
        foeGfxVkGraphImageResource imageResource;
        foeGfxVkGraphImageState imageState;
    };

    ImportImageJobResources *pJobResources = new (std::nothrow) ImportImageJobResources;
    if (pJobResources == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_OUT_OF_MEMORY);

    pJobResources->imageResource = foeGfxVkGraphImageResource{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE,
        .pNext = nullptr,
        .image = image,
        .view = view,
        .format = format,
        .extent = extent,
    };

    pJobResources->imageState = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = layout,
        .subresourceRange =
            {
                .aspectMask = foeGfxVkFormatAspects(format),
                .baseMipLevel = 0,
                .levelCount = 1,
                .layerCount = 1,
            },
    };

    foeGfxVkRenderGraphFn freeDataFn = [=]() -> void { delete pJobResources; };

    foeGfxVkRenderGraphResourceCreateInfo resourceCI{
        .sType = FOE_NULL_HANDLE,
        .pName = pResourceName,
        .isMutable = isMutable,
        .pResourceData = &pJobResources->imageResource,
    };

    foeGfxVkRenderGraphResource newImageResource = FOE_NULL_HANDLE;
    result = foeGfxVkRenderGraphCreateResource(renderGraph, &resourceCI, &newImageResource);
    if (result.value != FOE_SUCCESS) {
        freeDataFn();
        return result;
    }

    // Add job to graph
    foeGfxVkRenderGraphResourceState resourceState = foeGfxVkRenderGraphResourceState{
        .mode = (isMutable) ? RENDER_GRAPH_RESOURCE_MODE_READ_WRITE
                            : RENDER_GRAPH_RESOURCE_MODE_READ_ONLY,
        .resource = newImageResource,
        .pIncomingState = nullptr,
        .pOutgoingState = (foeGfxVkRenderGraphStructure *)&pJobResources->imageState,
    };

    foeGfxVkRenderGraphJobInfo jobInfo{
        .resourceCount = 1,
        .pResources = &resourceState,
        .freeDataFn = freeDataFn,
        .name = pJobName,
        .required = false,
        .waitSemaphoreCount = (uint32_t)waitSemaphores.size(),
        .pWaitSemaphores = waitSemaphores.data(),
        .fence = fence,
    };

    result = foeGfxVkRenderGraphAddJob(renderGraph, &jobInfo, {}, {}, pRenderGraphJob);
    if (result.value != FOE_SUCCESS) {
        freeDataFn();
    } else {
        *pImportedImageResource = newImageResource;
    }

    return result;
}