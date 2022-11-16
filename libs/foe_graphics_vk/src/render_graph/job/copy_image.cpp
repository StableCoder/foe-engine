// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/render_graph/job/copy_image.hpp>

#include <foe/graphics/vk/render_graph/resource/image.hpp>
#include <foe/graphics/vk/session.h>

#include "../../result.h"
#include "../../vk_result.h"

#include <array>
#include <vector>

foeResultSet foeGfxVkCopyImageRenderJob(foeGfxVkRenderGraph renderGraph,
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
        // Copy Command
        VkImageCopy imageCopy{
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

        vkCmdCopyImage(commandBuffer, pSrcImageData->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       pDstImageData->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);

        return vk_to_foeResult(VK_SUCCESS);
    };

    // Resource Management
    auto *pFinalImageStates = new (std::nothrow) foeGfxVkGraphImageState[2];
    if (pFinalImageStates == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_OUT_OF_MEMORY);

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
    std::array<foeGfxVkRenderGraphResourceState, 2> resources{
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
        .resourceCount = resources.size(),
        .pResources = resources.data(),
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