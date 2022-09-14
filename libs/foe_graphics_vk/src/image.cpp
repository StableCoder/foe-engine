// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/image.h>

#include "queue_family.hpp"
#include "upload_buffer.hpp"
#include "upload_context.hpp"
#include "upload_request.hpp"
#include "vk_result.h"

#include <cmath>

extern "C" uint32_t foeGfxVkMipmapCount(VkExtent3D extent) {
    if (extent.width == 0U || extent.height == 0U || extent.depth == 0U) {
        return 0;
    }

    uint32_t levels = 1;
    while (extent.width > 1U || extent.height > 1U || extent.depth > 1U) {
        extent.width = std::max(1u, extent.width / 2);
        extent.height = std::max(1u, extent.height / 2);
        extent.depth = std::max(1u, extent.depth / 2);

        ++levels;
    }

    return levels;
}

extern "C" VkExtent3D foeGfxVkMipmapExtent(VkExtent3D extent, uint32_t mipLevel) {
    uint32_t divisor = 1U << mipLevel;

    return VkExtent3D{std::max(1U, static_cast<uint32_t>(std::floor(extent.width / divisor))),
                      std::max(1U, static_cast<uint32_t>(std::floor(extent.height / divisor))),
                      std::max(1U, static_cast<uint32_t>(std::floor(extent.depth / divisor)))};
}

extern "C" VkDeviceSize foeGfxVkExtentPixelCount(VkExtent3D extent, uint32_t mipLevels) {
    VkDeviceSize pelCount = extent.width * extent.height * extent.depth;

    for (uint32_t i = 1; i < mipLevels; ++i) {
        VkExtent3D mipExtent = foeGfxVkMipmapExtent(extent, i);
        pelCount += mipExtent.width * mipExtent.height * mipExtent.depth;
    }

    return pelCount;
}

extern "C" foeResultSet foeGfxVkRecordImageUploadBufferUploadCommands(
    foeGfxUploadContext uploadContext,
    VkImageSubresourceRange const *pSubresourceRange,
    uint32_t copyRegionCount,
    VkBufferImageCopy const *pCopyRegions,
    foeGfxUploadBuffer srcBuffer,
    VkImage dstImage,
    VkAccessFlags dstAccessFlags,
    VkImageLayout dstImageLayout,
    foeGfxUploadRequest *pUploadRequst) {
    auto *pSrcBuffer = upload_buffer_from_handle(srcBuffer);

    return foeGfxVkRecordImageBufferUploadCommands(
        uploadContext, pSubresourceRange, copyRegionCount, pCopyRegions, pSrcBuffer->buffer,
        dstImage, dstAccessFlags, dstImageLayout, pUploadRequst);
}

extern "C" foeResultSet foeGfxVkRecordImageBufferUploadCommands(
    foeGfxUploadContext uploadContext,
    VkImageSubresourceRange const *pSubresourceRange,
    uint32_t copyRegionCount,
    VkBufferImageCopy const *pCopyRegions,
    VkBuffer srcBuffer,
    VkImage dstImage,
    VkAccessFlags dstAccessFlags,
    VkImageLayout dstImageLayout,
    foeGfxUploadRequest *pUploadRequst) {
    auto *pUploadContext = upload_context_from_handle(uploadContext);

    VkResult vkResult;
    foeGfxVkUploadRequest *uploadData{nullptr};

    vkResult = foeGfxVkCreateUploadData(pUploadContext->device, pUploadContext->srcCommandPool,
                                        pUploadContext->dstCommandPool, &uploadData);
    if (vkResult != VK_SUCCESS) {
        return vk_to_foeResult(vkResult);
    }

    { // Begin Recording
        VkCommandBufferBeginInfo cmdBufBI{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };

        if (uploadData->srcCmdBuffer != VK_NULL_HANDLE) {
            vkResult = vkBeginCommandBuffer(uploadData->srcCmdBuffer, &cmdBufBI);
            if (vkResult != VK_SUCCESS) {
                goto RECORDING_FAILED;
            }
        }

        vkResult = vkBeginCommandBuffer(uploadData->dstCmdBuffer, &cmdBufBI);
        if (vkResult != VK_SUCCESS) {
            goto RECORDING_FAILED;
        }
    }

    if (copyRegionCount > 0) {
        // Prepare the destination images for writing
        VkImageMemoryBarrier imgMemBarrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = dstImage,
            .subresourceRange = *pSubresourceRange,
        };

        // If there's no src buffer, use the dst buffer
        auto commandBuffer = (uploadData->srcCmdBuffer != VK_NULL_HANDLE)
                                 ? uploadData->srcCmdBuffer
                                 : uploadData->dstCmdBuffer;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1,
                             &imgMemBarrier);

        vkCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, copyRegionCount, pCopyRegions);
    }

    { // Change destination image for shader read only optimal
        auto srcQueueFamily = (uploadData->srcCmdBuffer != VK_NULL_HANDLE)
                                  ? pUploadContext->pSrcQueueFamily->family
                                  : VK_QUEUE_FAMILY_IGNORED;

        auto dstQueueFamily = (uploadData->srcCmdBuffer != VK_NULL_HANDLE)
                                  ? pUploadContext->pDstQueueFamily->family
                                  : VK_QUEUE_FAMILY_IGNORED;

        VkImageMemoryBarrier imgMemBarrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .dstAccessMask = dstAccessFlags,
            .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .newLayout = dstImageLayout,
            .srcQueueFamilyIndex = srcQueueFamily,
            .dstQueueFamilyIndex = dstQueueFamily,
            .image = dstImage,
            .subresourceRange = *pSubresourceRange,
        };

        if (uploadData->srcCmdBuffer != VK_NULL_HANDLE) {
            vkCmdPipelineBarrier(uploadData->srcCmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1,
                                 &imgMemBarrier);
        }

        vkCmdPipelineBarrier(uploadData->dstCmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1,
                             &imgMemBarrier);
    }

    { // End Recording
        if (uploadData->srcCmdBuffer != VK_NULL_HANDLE) {
            vkResult = vkEndCommandBuffer(uploadData->srcCmdBuffer);
            if (vkResult != VK_SUCCESS) {
                goto RECORDING_FAILED;
            }
        }

        vkResult = vkEndCommandBuffer(uploadData->dstCmdBuffer);
        if (vkResult != VK_SUCCESS) {
            goto RECORDING_FAILED;
        }
    }

RECORDING_FAILED:
    if (vkResult == VK_SUCCESS) {
        *pUploadRequst = upload_request_to_handle(uploadData);
    } else {
        foeGfxVkDestroyUploadRequest(pUploadContext->device, uploadData);
    }

    return vk_to_foeResult(vkResult);
}

extern "C" VkImageAspectFlags foeGfxVkFormatAspects(VkFormat format) {
    switch (format) {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_D32_SFLOAT:
        return VK_IMAGE_ASPECT_DEPTH_BIT;

    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

    case VK_FORMAT_UNDEFINED:
        return VkImageAspectFlags{};

    default:
        return VK_IMAGE_ASPECT_COLOR_BIT;
    }
}
