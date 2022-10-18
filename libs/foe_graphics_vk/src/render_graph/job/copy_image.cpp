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
                                        VkImageLayout srcFinalLayout,
                                        foeGfxVkRenderGraphResource dstImage,
                                        VkImageLayout dstFinalLayout,
                                        CopyJobUsedResources *pResourcesOut) {
    // Check that resources are correct types
    auto const *pSrcImageData = (foeGfxVkGraphImageResource const *)foeGfxVkGraphFindStructure(
        srcImage.pResourceData, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE);
    auto const *pDstImageData = (foeGfxVkGraphImageResource const *)foeGfxVkGraphFindStructure(
        dstImage.pResourceData, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE);

    if (pSrcImageData == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_SOURCE_NOT_IMAGE);
    if (pDstImageData == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_DESTINATION_NOT_IMAGE);
    if (!pDstImageData->isMutable)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_DESTINATION_NOT_MUTABLE);

    // Get the last states of the images
    auto const *pSrcImageState = (foeGfxVkGraphImageState const *)foeGfxVkGraphFindStructure(
        srcImage.pResourceState, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE);
    auto const *pDstImageState = (foeGfxVkGraphImageState const *)foeGfxVkGraphFindStructure(
        dstImage.pResourceState, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE);

    if (pSrcImageState == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_SOURCE_NO_STATE);
    if (pDstImageState == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_DESTINATION_NO_STATE);

    // Proceed with job
    auto jobFn = [=](foeGfxSession, foeGfxDelayedCaller,
                     VkCommandBuffer commandBuffer) -> foeResultSet {
        // Transition image layout/mask states of incoming
        VkImageSubresourceRange subresourceRange{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .layerCount = 1,
        };
        uint32_t numBarriers = 0;
        VkImageMemoryBarrier imgMemBarrier[2] = {};

        if (pSrcImageState->layout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
            imgMemBarrier[numBarriers] = VkImageMemoryBarrier{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask = foeGfxVkDetermineAccessFlags(pSrcImageState->layout),
                .dstAccessMask = foeGfxVkDetermineAccessFlags(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL),
                .oldLayout = pSrcImageState->layout,
                .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = pSrcImageData->image,
                .subresourceRange = subresourceRange,
            };
            ++numBarriers;
        }
        if (pDstImageState->layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            imgMemBarrier[numBarriers] = VkImageMemoryBarrier{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask = foeGfxVkDetermineAccessFlags(pDstImageState->layout),
                .dstAccessMask = foeGfxVkDetermineAccessFlags(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL),
                .oldLayout = pDstImageState->layout,
                .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = pDstImageData->image,
                .subresourceRange = subresourceRange,
            };
            ++numBarriers;
        }

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr,
                             numBarriers, imgMemBarrier);

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

        // Transition images layout/masks of outgoing
        numBarriers = 0;

        if (srcFinalLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
            imgMemBarrier[numBarriers] = VkImageMemoryBarrier{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask = foeGfxVkDetermineAccessFlags(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL),
                .dstAccessMask = foeGfxVkDetermineAccessFlags(srcFinalLayout),
                .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                .newLayout = srcFinalLayout,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = pSrcImageData->image,
                .subresourceRange = subresourceRange,
            };
            ++numBarriers;
        }
        if (dstFinalLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            imgMemBarrier[numBarriers] = VkImageMemoryBarrier{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask = foeGfxVkDetermineAccessFlags(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL),
                .dstAccessMask = foeGfxVkDetermineAccessFlags(dstFinalLayout),
                .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .newLayout = dstFinalLayout,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = pDstImageData->image,
                .subresourceRange = subresourceRange,
            };
            ++numBarriers;
        }

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr,
                             numBarriers, imgMemBarrier);

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

    foeGfxVkRenderGraphFn freeDataFn = [=]() -> void { delete[] pFinalImageStates; };

    // Add job to graph
    std::array<foeGfxVkRenderGraphResource const, 2> resourcesIn{srcImage, dstImage};
    std::array<bool const, 2> resourcesInReadOnly{true, false};
    foeGfxVkRenderGraphJob renderGraphJob;

    foeGfxVkRenderGraphJobInfo jobInfo{
        .resourceCount = resourcesIn.size(),
        .pResourcesIn = resourcesIn.data(),
        .pResourcesInReadOnly = resourcesInReadOnly.data(),
        .freeDataFn = freeDataFn,
        .name = pJobName,
        .required = false,
        .fence = fence,
    };

    foeResultSet result =
        foeGfxVkRenderGraphAddJob(renderGraph, &jobInfo, {}, std::move(jobFn), &renderGraphJob);
    if (result.value != FOE_SUCCESS) {
        freeDataFn();
    } else {

        // Outgoing resources
        *pResourcesOut = CopyJobUsedResources{
            .srcImage =
                {
                    .provider = renderGraphJob,
                    .pResourceData = srcImage.pResourceData,
                    .pResourceState =
                        reinterpret_cast<foeGfxVkRenderGraphStructure const *>(pFinalImageStates),
                },
            .dstImage =
                {
                    .provider = renderGraphJob,
                    .pResourceData = dstImage.pResourceData,
                    .pResourceState = reinterpret_cast<foeGfxVkRenderGraphStructure const *>(
                        pFinalImageStates + 1),
                },
        };
    }

    return result;
}