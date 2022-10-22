// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/render_graph/job/export_image.hpp>

#include <foe/graphics/vk/render_graph/resource/image.hpp>
#include <foe/graphics/vk/session.h>

#include "../../result.h"
#include "../../vk_result.h"

foeResultSet foeGfxVkExportImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                          char const *pJobName,
                                          VkFence fence,
                                          foeGfxVkRenderGraphResource resource,
                                          VkImageLayout requiredLayout,
                                          std::vector<VkSemaphore> signalSemaphores) {
    // Check that this is an image resource
    auto const *pImageData = (foeGfxVkGraphImageResource const *)foeGfxVkGraphFindStructure(
        foeGfxVkRenderGraphGetResourceData(resource.resource),
        RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE);

    if (pImageData == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_EXPORT_IMAGE_RESOURCE_NOT_IMAGE);

    // Check that this is in the correct/desired state
    auto const *pImageState = (foeGfxVkGraphImageState const *)foeGfxVkGraphFindStructure(
        resource.pResourceState, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE);

    if (pImageState == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_EXPORT_IMAGE_RESOURCE_NO_STATE);
    if (pImageState->layout != requiredLayout)
        std::abort();

    // Add job to graph
    bool readOnly = true;
    foeGfxVkRenderGraphJob renderGraphJob;

    foeGfxVkRenderGraphJobInfo jobInfo{
        .resourceCount = 1,
        .pResourcesIn = &resource,
        .pResourcesInReadOnly = &readOnly,
        .name = pJobName,
        .required = true,
        .signalSemaphoreCount = (uint32_t)signalSemaphores.size(),
        .pSignalSemaphores = signalSemaphores.data(),
        .fence = fence,
    };

    return foeGfxVkRenderGraphAddJob(renderGraph, &jobInfo, {}, {}, &renderGraphJob);
}