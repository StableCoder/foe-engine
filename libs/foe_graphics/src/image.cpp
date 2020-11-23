/*
    Copyright (C) 2020 George Cave.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <foe/graphics/image.hpp>

#include <foe/graphics/resource_uploader.hpp>
#include <foe/graphics/upload_data.hpp>

#include <cmath>

uint32_t maxMipmapCount(VkExtent3D extent) noexcept {
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

VkExtent3D mipmapExtent(VkExtent3D extent, uint32_t mipLevel) noexcept {
    uint32_t divisor = 1U << mipLevel;

    return VkExtent3D{std::max(1U, static_cast<uint32_t>(std::floor(extent.width / divisor))),
                      std::max(1U, static_cast<uint32_t>(std::floor(extent.height / divisor))),
                      std::max(1U, static_cast<uint32_t>(std::floor(extent.depth / divisor)))};
}

VkDeviceSize pixelCount(VkExtent3D extent, uint32_t mipLevels) noexcept {
    VkDeviceSize pelCount = extent.width * extent.height * extent.depth;

    for (uint32_t i = 1; i < mipLevels; ++i) {
        VkExtent3D mipExtent = mipmapExtent(extent, i);
        pelCount += mipExtent.width * mipExtent.height * mipExtent.depth;
    }

    return pelCount;
}

VkResult recordImageUploadCommands(foeResourceUploader *pResourceUploader,
                                   VkImageSubresourceRange const *pSubresourceRange,
                                   uint32_t copyRegionCount,
                                   VkBufferImageCopy const *pCopyRegions,
                                   VkBuffer srcBuffer,
                                   VkImage dstImage,
                                   VkAccessFlags dstAccessFlags,
                                   VkImageLayout dstImageLayout,
                                   foeUploadData *pUploadData) {
    VkResult res;
    foeUploadData uploadData;

    res = createUploadData(pResourceUploader, &uploadData);
    if (res != VK_SUCCESS) {
        return res;
    }

    { // Begin Recording
        VkCommandBufferBeginInfo cmdBufBI{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };

        if (uploadData.srcCmdBuffer != VK_NULL_HANDLE) {
            res = vkBeginCommandBuffer(uploadData.srcCmdBuffer, &cmdBufBI);
            if (res != VK_SUCCESS) {
                goto RECORDING_FAILED;
            }
        }

        res = vkBeginCommandBuffer(uploadData.dstCmdBuffer, &cmdBufBI);
        if (res != VK_SUCCESS) {
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
        auto commandBuffer = (uploadData.srcCmdBuffer != VK_NULL_HANDLE) ? uploadData.srcCmdBuffer
                                                                         : uploadData.dstCmdBuffer;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1,
                             &imgMemBarrier);

        vkCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, copyRegionCount, pCopyRegions);
    }

    { // Change destination image for shader read only optimal
        auto srcQueueFamily = (uploadData.srcCmdBuffer != VK_NULL_HANDLE)
                                  ? pResourceUploader->srcQueueFamily->family
                                  : VK_QUEUE_FAMILY_IGNORED;

        auto dstQueueFamily = (uploadData.srcCmdBuffer != VK_NULL_HANDLE)
                                  ? pResourceUploader->dstQueueFamily->family
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

        if (uploadData.srcCmdBuffer != VK_NULL_HANDLE) {
            vkCmdPipelineBarrier(uploadData.srcCmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1,
                                 &imgMemBarrier);
        }

        vkCmdPipelineBarrier(uploadData.dstCmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1,
                             &imgMemBarrier);
    }

    { // End Recording
        if (uploadData.srcCmdBuffer != VK_NULL_HANDLE) {
            res = vkEndCommandBuffer(uploadData.srcCmdBuffer);
            if (res != VK_SUCCESS) {
                goto RECORDING_FAILED;
            }
        }

        res = vkEndCommandBuffer(uploadData.dstCmdBuffer);
        if (res != VK_SUCCESS) {
            goto RECORDING_FAILED;
        }
    }

RECORDING_FAILED:
    if (res == VK_SUCCESS) {
        *pUploadData = uploadData;
    } else {
        uploadData.destroy(pResourceUploader->device);
    }

    return res;
}