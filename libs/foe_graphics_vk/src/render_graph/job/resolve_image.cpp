// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/render_graph/job/resolve_image.hpp>

#include <foe/graphics/vk/render_graph/resource/image.hpp>
#include <foe/graphics/vk/session.h>

#include "../../result.h"
#include "../../vk_result.h"

#include <array>
#include <vector>

foeResultSet foeGfxVkResolveImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                           char const *pJobName,
                                           VkFence fence,
                                           foeGfxVkRenderGraphResource srcImage,
                                           VkImageLayout srcFinalLayout,
                                           foeGfxVkRenderGraphResource dstImage,
                                           VkImageLayout dstFinalLayout,
                                           ResolveJobUsedResources *pResourcesOut) {
    // Check that resources are images and the destination is mutable
    auto const *pSrcImageData = (foeGfxVkGraphImageResource const *)foeGfxVkGraphFindStructure(
        foeGfxVkRenderGraphGetResourceData(srcImage.resource),
        RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE);
    auto const *pDstImageData = (foeGfxVkGraphImageResource const *)foeGfxVkGraphFindStructure(
        foeGfxVkRenderGraphGetResourceData(dstImage.resource),
        RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE);

    if (pSrcImageData == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_SOURCE_NOT_IMAGE);
    if (pDstImageData == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_DESTINATION_NOT_IMAGE);
    if (!foeGfxVkRenderGraphGetResourceIsMutable(dstImage.resource))
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_DESTINATION_NOT_MUTABLE);

    // Get the last states of the images
    auto const *pSrcImageState = (foeGfxVkGraphImageState const *)foeGfxVkGraphFindStructure(
        srcImage.pResourceState, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE);
    auto const *pDstImageState = (foeGfxVkGraphImageState const *)foeGfxVkGraphFindStructure(
        dstImage.pResourceState, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE);

    if (pSrcImageState == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_SOURCE_NO_STATE);
    if (pDstImageState == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_DESTINATION_NO_STATE);

    // Proceed with the job
    auto jobFn = [=](foeGfxSession, foeGfxDelayedCaller,
                     VkCommandBuffer commandBuffer) -> foeResultSet {
        // Resolve Command
        VkImageResolve resolveRegion{
            .srcSubresource =
                VkImageSubresourceLayers{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            .srcOffset = {},
            .dstSubresource =
                VkImageSubresourceLayers{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            .dstOffset = {},
            .extent =
                {
                    .width = pSrcImageData->extent.width,
                    .height = pSrcImageData->extent.height,
                    .depth = 1,
                },
        };

        vkCmdResolveImage(commandBuffer, pSrcImageData->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                          pDstImageData->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                          &resolveRegion);
        return vk_to_foeResult(VK_SUCCESS);
    };

    // Resource Management
    auto *pFinalImageStates = new (std::nothrow) foeGfxVkGraphImageState[2];
    if (pFinalImageStates == nullptr) {
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_OUT_OF_MEMORY);
    }

    pFinalImageStates[0] = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = srcFinalLayout,
    };
    pFinalImageStates[1] = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = dstFinalLayout,
    };

    auto *pImageStates = new (std::nothrow) foeGfxVkGraphImageState[4];
    if (pImageStates == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_OUT_OF_MEMORY);

    // Source incoming
    pImageStates[0] = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .layerCount = 1,
            },
    };
    // Source outgoing
    pImageStates[1] = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .layerCount = 1,
            },
    };

    // Destination incoming
    pImageStates[2] = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .layerCount = 1,
            },
    };
    // Destination outgoing
    pImageStates[3] = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .layerCount = 1,
            },
    };

    foeGfxVkRenderGraphFn freeDataFn = [=]() -> void {
        delete[] pFinalImageStates;
        delete[] pImageStates;
    };

    // Add job to graph
    std::array<foeGfxVkRenderGraphResourceState, 2> resourceStates{
        foeGfxVkRenderGraphResourceState{
            .upstreamJobCount = 1,
            .pUpstreamJobs = &srcImage.provider,
            .mode = RENDER_GRAPH_RESOURCE_MODE_READ_ONLY,
            .resource = srcImage.resource,
            .pIncomingState = (foeGfxVkRenderGraphStructure *)pImageStates,
            .pOutgoingState = (foeGfxVkRenderGraphStructure *)(pImageStates + 1),
        },
        foeGfxVkRenderGraphResourceState{
            .upstreamJobCount = 1,
            .pUpstreamJobs = &dstImage.provider,
            .mode = RENDER_GRAPH_RESOURCE_MODE_READ_WRITE,
            .resource = dstImage.resource,
            .pIncomingState = (foeGfxVkRenderGraphStructure *)(pImageStates + 2),
            .pOutgoingState = (foeGfxVkRenderGraphStructure *)(pImageStates + 3),
        },
    };

    foeGfxVkRenderGraphJobInfo jobInfo{
        .resourceCount = resourceStates.size(),
        .pResources = resourceStates.data(),
        .freeDataFn = freeDataFn,
        .name = pJobName,
        .required = false,
        .fence = fence,
    };

    foeGfxVkRenderGraphJob renderGraphJob;
    foeResultSet result =
        foeGfxVkRenderGraphAddJob(renderGraph, &jobInfo, {}, std::move(jobFn), &renderGraphJob);
    if (result.value != FOE_SUCCESS) {
        freeDataFn();

        return result;
    }

    // Outgoing resources
    *pResourcesOut = ResolveJobUsedResources{
        .srcImage =
            {
                .provider = renderGraphJob,
                .resource = srcImage.resource,
                .pResourceState =
                    reinterpret_cast<foeGfxVkRenderGraphStructure const *>(pFinalImageStates),
            },
        .dstImage =
            {
                .provider = renderGraphJob,
                .resource = dstImage.resource,
                .pResourceState =
                    reinterpret_cast<foeGfxVkRenderGraphStructure const *>(pFinalImageStates + 1),
            },
    };

    return to_foeResult(FOE_GRAPHICS_VK_SUCCESS);
}