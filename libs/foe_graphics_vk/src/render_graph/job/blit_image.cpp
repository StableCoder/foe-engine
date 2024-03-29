// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/render_graph/job/blit_image.hpp>

#include <foe/graphics/vk/render_graph/resource/image.hpp>
#include <foe/graphics/vk/session.h>

#include "../../result.h"
#include "../../vk_result.h"

#include <array>
#include <vector>

foeResultSet foeGfxVkBlitImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                        char const *pJobName,
                                        VkFence fence,
                                        foeGfxVkRenderGraphResource srcImage,
                                        uint32_t srcImageUpstreamJobCount,
                                        foeGfxVkRenderGraphJob const *pSrcImageUpstreamJobs,
                                        VkImageLayout srcFinalLayout,
                                        foeGfxVkRenderGraphResource dstImage,
                                        uint32_t dstImageUpstreamJobCount,
                                        foeGfxVkRenderGraphJob const *pDstImageUpstreamJobs,
                                        VkImageLayout dstFinalLayout,
                                        VkFilter filter,
                                        foeGfxVkRenderGraphJob *pRenderGraphJob) {
    // Check that resources are correct types
    auto const *pSrcImageData = (foeGfxVkGraphImageResource const *)foeGfxVkGraphFindStructure(
        foeGfxVkRenderGraphGetResourceData(srcImage), RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE);
    auto const *pDstImageData = (foeGfxVkGraphImageResource const *)foeGfxVkGraphFindStructure(
        foeGfxVkRenderGraphGetResourceData(dstImage), RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE);

    if (pSrcImageData == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_SOURCE_NOT_IMAGE);
    if (pDstImageData == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_DESTINATION_NOT_IMAGE);
    if (!foeGfxVkRenderGraphGetResourceIsMutable(dstImage))
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_DESTINATION_NOT_MUTABLE);

    // Proceed with job
    auto jobFn = [=](foeGfxSession, foeGfxDelayedCaller,
                     VkCommandBuffer commandBuffer) -> foeResultSet {
        // Blit Command
        VkImageBlit imageBlit{
            .srcSubresource =
                VkImageSubresourceLayers{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            .srcOffsets = {{},
                           VkOffset3D{.x = (int32_t)pSrcImageData->extent.width,
                                      .y = (int32_t)pSrcImageData->extent.height,
                                      .z = 1}},
            .dstSubresource =
                VkImageSubresourceLayers{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            .dstOffsets = {{},
                           VkOffset3D{.x = (int32_t)pDstImageData->extent.width,
                                      .y = (int32_t)pDstImageData->extent.height,
                                      .z = 1}},
        };

        vkCmdBlitImage(commandBuffer, pSrcImageData->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       pDstImageData->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit,
                       filter);

        return vk_to_foeResult(VK_SUCCESS);
    };

    // Resource Management
    auto *pFinalImageStates = new (std::nothrow) foeGfxVkGraphImageState[2];
    if (pFinalImageStates == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_OUT_OF_MEMORY);

    pFinalImageStates[0] = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = srcFinalLayout,
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .layerCount = 1,
            },
    };
    pFinalImageStates[1] = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = dstFinalLayout,
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .layerCount = 1,
            },
    };

    auto *pImageStates = new (std::nothrow) foeGfxVkGraphImageState[4];
    if (pImageStates == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_OUT_OF_MEMORY);

    // Source incoming
    pImageStates[0] = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    };
    // Source outgoing
    pImageStates[1] = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    };

    // Destination incoming
    pImageStates[2] = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    };
    // Destination outgoing
    pImageStates[3] = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    };

    foeGfxVkRenderGraphFn freeDataFn = [=]() -> void {
        delete[] pFinalImageStates;
        delete[] pImageStates;
    };

    // Add job to graph
    std::array<foeGfxVkRenderGraphResourceState, 2> resourceStates{
        foeGfxVkRenderGraphResourceState{
            .upstreamJobCount = srcImageUpstreamJobCount,
            .pUpstreamJobs = pSrcImageUpstreamJobs,
            .mode = RENDER_GRAPH_RESOURCE_MODE_READ_ONLY,
            .resource = srcImage,
            .pIncomingState = (foeGfxVkRenderGraphStructure *)pImageStates,
            .pOutgoingState = (foeGfxVkRenderGraphStructure *)(pImageStates + 1),
        },
        foeGfxVkRenderGraphResourceState{
            .upstreamJobCount = dstImageUpstreamJobCount,
            .pUpstreamJobs = pDstImageUpstreamJobs,
            .mode = RENDER_GRAPH_RESOURCE_MODE_READ_WRITE,
            .resource = dstImage,
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

    foeResultSet result =
        foeGfxVkRenderGraphAddJob(renderGraph, &jobInfo, {}, std::move(jobFn), pRenderGraphJob);
    if (result.value != FOE_SUCCESS) {
        freeDataFn();
    }

    return result;
}