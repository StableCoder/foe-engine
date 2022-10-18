// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/render_graph/job/import_image.hpp>

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
                                          foeGfxVkRenderGraphResource *pResourcesOut) {
    // Resource management
    auto *pImportedImage = new (std::nothrow) foeGfxVkGraphImageResource{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE,
        .pNext = nullptr,
        .name = pResourceName,
        .image = image,
        .view = view,
        .format = format,
        .extent = extent,
        .isMutable = isMutable,
    };
    if (pImportedImage == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_OUT_OF_MEMORY);

    auto *pImageState = new (std::nothrow) foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = layout,
    };
    if (pImageState == nullptr) {
        delete pImportedImage;
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_OUT_OF_MEMORY);
    }

    foeGfxVkRenderGraphFn freeDataFn = [=]() -> void {
        delete pImportedImage;
        delete pImageState;
    };

    // Add job to graph
    foeGfxVkRenderGraphJob renderGraphJob;

    foeGfxVkRenderGraphJobInfo jobInfo{
        .freeDataFn = freeDataFn,
        .name = pJobName,
        .required = false,
        .waitSemaphoreCount = (uint32_t)waitSemaphores.size(),
        .pWaitSemaphores = waitSemaphores.data(),
        .fence = fence,
    };

    foeResultSet result = foeGfxVkRenderGraphAddJob(renderGraph, &jobInfo, {}, {}, &renderGraphJob);
    if (result.value != FOE_SUCCESS) {
        freeDataFn();
    } else {
        // Outgoing resources
        *pResourcesOut = foeGfxVkRenderGraphResource{
            .provider = renderGraphJob,
            .pResourceData = reinterpret_cast<foeGfxVkRenderGraphStructure const *>(pImportedImage),
            .pResourceState = reinterpret_cast<foeGfxVkRenderGraphStructure const *>(pImageState),
        };
    }

    return result;
}