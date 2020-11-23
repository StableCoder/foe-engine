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

#include <foe/graphics/error_images.hpp>

#include <foe/graphics/image.hpp>
#include <foe/graphics/upload_data.hpp>

#include "format.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <vector>

namespace {

void fillErrorDepthData(uint32_t extent, uint32_t numCheckSquare, float *pData) {
    uint32_t squareSize = extent / numCheckSquare;
    if (squareSize == 0) {
        squareSize = extent / 2U;
    }
    if (squareSize == 0) {
        squareSize = 1U;
    }

    for (uint32_t x = 0; x < extent; ++x) {
        for (uint32_t y = 0; y < extent; ++y) {
            if ((x / squareSize) % 2 == 0) {
                if ((y / squareSize) % 2 == 0) {
                    *pData++ = 0.f;
                } else {
                    *pData++ = 1.f;
                }
            } else {
                if ((y / squareSize) % 2 == 0) {
                    *pData++ = 1.f;
                } else {
                    *pData++ = 0.f;
                }
            }
        }
    }
}

void fillErrorStencilData(uint32_t extent, uint32_t numCheckSquare, uint8_t *pData) {
    uint32_t squareSize = extent / numCheckSquare;
    if (squareSize == 0) {
        squareSize = extent / 2U;
    }
    if (squareSize == 0) {
        squareSize = 1U;
    }

    for (uint32_t x = 0; x < extent; ++x) {
        for (uint32_t y = 0; y < extent; ++y) {
            if ((x / squareSize) % 2 == 0) {
                if ((y / squareSize) % 2 == 0) {
                    *pData++ = 0x00;
                } else {
                    *pData++ = 0xFF;
                }
            } else {
                if ((y / squareSize) % 2 == 0) {
                    *pData++ = 0xFF;
                } else {
                    *pData++ = 0x00;
                }
            }
        }
    }
}

} // namespace

VkResult foeCreateErrorDepthStencilImage(foeResourceUploader *pResourceUploader,
                                         uint32_t numMipLevels,
                                         uint32_t numCheckSquares,
                                         VmaAllocation *pAlloc,
                                         VkImage *pImage,
                                         VkImageView *pImageDepthView,
                                         VkImageView *pImageStencilView,
                                         VkSampler *pSampler) {
    VkResult res;
    constexpr VkFormat format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    uint32_t const maxExtent = std::pow(2U, numMipLevels - 1U);
    VkExtent3D extent = VkExtent3D{maxExtent, maxExtent, 1U};
    foeUploadData uploadData{};

    VmaAllocation stagingAlloc{VK_NULL_HANDLE};
    VkBuffer stagingBuffer{VK_NULL_HANDLE};

    std::vector<VkBufferImageCopy> copyRegions;

    VmaAllocation alloc{VK_NULL_HANDLE};
    VkImage image{VK_NULL_HANDLE};
    VkImageView depthView{VK_NULL_HANDLE};
    VkImageView stencilView{VK_NULL_HANDLE};
    VkSampler sampler{VK_NULL_HANDLE};

    { // Staging Buffer
        uint32_t dataByteSize = pixelCount(extent, numMipLevels);
        dataByteSize *=
            bytesPerPixel(format, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

        VkBufferCreateInfo bufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = dataByteSize,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        VmaAllocationCreateInfo allocCI{
            .usage = VMA_MEMORY_USAGE_CPU_ONLY,
        };

        res = vmaCreateBuffer(pResourceUploader->allocator, &bufferCI, &allocCI, &stagingBuffer,
                              &stagingAlloc, nullptr);
        if (res != VK_SUCCESS) {
            goto CREATE_FAILED;
        }
    }

    { // Image
        VkImageCreateInfo imageCI{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = format,
            .extent = VkExtent3D{maxExtent, maxExtent, 1U},
            .mipLevels = numMipLevels,
            .arrayLayers = 1U,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        };

        VmaAllocationCreateInfo allocCI{
            .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        };

        res = vmaCreateImage(pResourceUploader->allocator, &imageCI, &allocCI, &image, &alloc,
                             nullptr);
        if (res != VK_SUCCESS) {
            goto CREATE_FAILED;
        }
    }

    { // Map data in
        uint8_t *pData;
        VkDeviceSize offset = 0;
        copyRegions.resize(numMipLevels * 2);
        auto const depth_bpp = bytesPerPixel(format, VK_IMAGE_ASPECT_DEPTH_BIT);
        auto const stencil_bpp = bytesPerPixel(format, VK_IMAGE_ASPECT_STENCIL_BIT);

        res = vmaMapMemory(pResourceUploader->allocator, stagingAlloc,
                           reinterpret_cast<void **>(&pData));
        if (res != VK_SUCCESS) {
            goto CREATE_FAILED;
        }

        for (uint32_t i = 0; i < numMipLevels; ++i) {
            auto const mipExtent = mipmapExtent(extent, i);

            // Depth
            fillErrorDepthData(mipExtent.width, numCheckSquares,
                               reinterpret_cast<float *>(pData + offset));

            copyRegions[i * 2 + 0] = VkBufferImageCopy{
                .bufferOffset = offset,
                .imageSubresource =
                    {
                        .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                        .mipLevel = i,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                    },
                .imageExtent = mipExtent,
            };

            offset += mipExtent.width * mipExtent.height * mipExtent.depth * depth_bpp;

            // Stencil
            fillErrorStencilData(mipExtent.width, numCheckSquares, pData + offset);

            copyRegions[i * 2 + 1] = VkBufferImageCopy{
                .bufferOffset = offset,
                .imageSubresource =
                    {
                        .aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT,
                        .mipLevel = i,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                    },
                .imageExtent = mipExtent,
            };

            offset += mipExtent.width * mipExtent.height * mipExtent.depth * stencil_bpp;
        }

        vmaUnmapMemory(pResourceUploader->allocator, stagingAlloc);
    }

    {
        // Record Upload Command
        VkImageSubresourceRange subresourceRange{
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
            .baseMipLevel = 0,
            .levelCount = numMipLevels,
            .layerCount = 1,
        };

        recordImageUploadCommands(pResourceUploader, &subresourceRange, copyRegions.size(),
                                  copyRegions.data(), stagingBuffer, image,
                                  VK_ACCESS_SHADER_READ_BIT,
                                  VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, &uploadData);
        if (res != VK_SUCCESS) {
            goto SUBMIT_FAILED;
        }

        res = submitUploadDataCommands(pResourceUploader, &uploadData);
        if (res != VK_SUCCESS) {
            goto SUBMIT_FAILED;
        }
    }

    { // Image View
        VkImageViewCreateInfo viewCI{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format,
            .components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B,
                           VK_COMPONENT_SWIZZLE_A},
            .subresourceRange =
                VkImageSubresourceRange{
                    .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                    .baseMipLevel = 0,
                    .levelCount = numMipLevels,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
        };

        // Depth
        res = vkCreateImageView(pResourceUploader->device, &viewCI, nullptr, &depthView);
        if (res != VK_SUCCESS) {
            goto CREATE_FAILED;
        }

        // Stencil
        viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        res = vkCreateImageView(pResourceUploader->device, &viewCI, nullptr, &stencilView);
        if (res != VK_SUCCESS) {
            goto CREATE_FAILED;
        }
    }

    { // Sampler
        VkSamplerCreateInfo samplerCI{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_NEAREST,
            .minFilter = VK_FILTER_NEAREST,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .mipLodBias = 0.f,
            .anisotropyEnable = VK_FALSE,
            .minLod = 0.f,
            .maxLod = static_cast<float>(numMipLevels - 1),
        };

        res = vkCreateSampler(pResourceUploader->device, &samplerCI, nullptr, &sampler);
        if (res != VK_SUCCESS) {
            goto CREATE_FAILED;
        }
    }

CREATE_FAILED:
    if (uploadData.dstFence != VK_NULL_HANDLE) {
        VkResult fenceStatus = VK_NOT_READY;
        while (fenceStatus == VK_NOT_READY) {
            fenceStatus = vkGetFenceStatus(pResourceUploader->device, uploadData.dstFence);
        }
        if (fenceStatus != VK_SUCCESS) {
            res = fenceStatus;
        }
    }

SUBMIT_FAILED: // Skips waiting for destination queue to complete if it failed to submit
    uploadData.destroy(pResourceUploader->device);

    if (stagingBuffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(pResourceUploader->allocator, stagingBuffer, stagingAlloc);
    }

    if (res == VK_SUCCESS) {
        *pImage = image;
        *pAlloc = alloc;
        *pImageDepthView = depthView;
        *pImageStencilView = stencilView;
        *pSampler = sampler;
    } else {
        if (sampler != VK_NULL_HANDLE) {
            vkDestroySampler(pResourceUploader->device, sampler, nullptr);
        }

        if (stencilView != VK_NULL_HANDLE) {
            vkDestroyImageView(pResourceUploader->device, stencilView, nullptr);
        }

        if (depthView != VK_NULL_HANDLE) {
            vkDestroyImageView(pResourceUploader->device, depthView, nullptr);
        }

        if (image != VK_NULL_HANDLE) {
            vmaDestroyImage(pResourceUploader->allocator, image, alloc);
        }
    }

    return res;
}