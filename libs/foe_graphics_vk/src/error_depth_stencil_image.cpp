// Copyright (C) 2020-2024 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/error_images.h>

#include <foe/graphics/vk/format.h>
#include <foe/graphics/vk/image.h>

#include "result.h"
#include "upload_context.hpp"
#include "upload_request.hpp"
#include "vk_result.h"

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

extern "C" foeResultSet foeCreateErrorDepthStencilImage(foeGfxUploadContext uploadContext,
                                                        uint32_t numMipLevels,
                                                        uint32_t numCheckSquares,
                                                        VmaAllocation *pAlloc,
                                                        VkImage *pImage,
                                                        VkImageView *pImageDepthView,
                                                        VkImageView *pImageStencilView,
                                                        VkSampler *pSampler) {
    auto *pUploadContext = upload_context_from_handle(uploadContext);

    foeResultSet result = to_foeResult(FOE_GRAPHICS_VK_SUCCESS);
    VkResult vkResult{VK_SUCCESS};
    constexpr VkFormat format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    uint32_t const maxExtent = std::pow(2U, numMipLevels - 1U);
    VkExtent3D extent = VkExtent3D{maxExtent, maxExtent, 1U};
    foeGfxUploadRequest uploadRequest{FOE_NULL_HANDLE};

    VmaAllocation stagingAlloc{VK_NULL_HANDLE};
    VkBuffer stagingBuffer{VK_NULL_HANDLE};

    std::vector<VkBufferImageCopy> copyRegions;

    VmaAllocation alloc{VK_NULL_HANDLE};
    VkImage image{VK_NULL_HANDLE};
    VkImageView depthView{VK_NULL_HANDLE};
    VkImageView stencilView{VK_NULL_HANDLE};
    VkSampler sampler{VK_NULL_HANDLE};

    { // Staging Buffer
        VkDeviceSize dataByteSize = foeGfxVkExtentPixelCount(extent, numMipLevels);
        dataByteSize *=
            foeGfxVkBytesPerPixel(format, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

        VkBufferCreateInfo bufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = dataByteSize,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        VmaAllocationCreateInfo allocCI{
            .usage = VMA_MEMORY_USAGE_CPU_ONLY,
        };

        vkResult = vmaCreateBuffer(pUploadContext->allocator, &bufferCI, &allocCI, &stagingBuffer,
                                   &stagingAlloc, nullptr);
        if (vkResult != VK_SUCCESS) {
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

        vkResult =
            vmaCreateImage(pUploadContext->allocator, &imageCI, &allocCI, &image, &alloc, nullptr);
        if (vkResult != VK_SUCCESS) {
            goto CREATE_FAILED;
        }
    }

    { // Map data in
        uint8_t *pData;
        VkDeviceSize offset = 0;
        copyRegions.resize(numMipLevels * 2);
        auto const depth_bpp = foeGfxVkBytesPerPixel(format, VK_IMAGE_ASPECT_DEPTH_BIT);
        auto const stencil_bpp = foeGfxVkBytesPerPixel(format, VK_IMAGE_ASPECT_STENCIL_BIT);

        vkResult = vmaMapMemory(pUploadContext->allocator, stagingAlloc,
                                reinterpret_cast<void **>(&pData));
        if (vkResult != VK_SUCCESS) {
            goto CREATE_FAILED;
        }

        for (uint32_t i = 0; i < numMipLevels; ++i) {
            auto const mipExtent = foeGfxVkMipmapExtent(extent, i);

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

        vmaUnmapMemory(pUploadContext->allocator, stagingAlloc);
    }

    {
        // Record Upload Command
        VkImageSubresourceRange subresourceRange{
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
            .baseMipLevel = 0,
            .levelCount = numMipLevels,
            .layerCount = 1,
        };

        foeGfxVkRecordImageBufferUploadCommands(
            uploadContext, &subresourceRange, static_cast<uint32_t>(copyRegions.size()),
            copyRegions.data(), stagingBuffer, image, VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, &uploadRequest);
        if (vkResult != VK_SUCCESS) {
            goto SUBMIT_FAILED;
        }

        result = foeSubmitUploadDataCommands(uploadContext, uploadRequest);
        if (result.value != FOE_SUCCESS) {
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
        vkResult = vkCreateImageView(pUploadContext->device, &viewCI, nullptr, &depthView);
        if (vkResult != VK_SUCCESS) {
            goto CREATE_FAILED;
        }

        // Stencil
        viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        vkResult = vkCreateImageView(pUploadContext->device, &viewCI, nullptr, &stencilView);
        if (vkResult != VK_SUCCESS) {
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

        vkResult = vkCreateSampler(pUploadContext->device, &samplerCI, nullptr, &sampler);
        if (vkResult != VK_SUCCESS) {
            goto CREATE_FAILED;
        }
    }

CREATE_FAILED: {
    auto requestStatus = foeGfxGetUploadRequestStatus(uploadRequest);
    while (requestStatus == FOE_GFX_UPLOAD_REQUEST_STATUS_INCOMPLETE) {
        requestStatus = foeGfxGetUploadRequestStatus(uploadRequest);
    }
    if (requestStatus != FOE_GFX_UPLOAD_REQUEST_STATUS_COMPLETE) {
        vkResult = VK_ERROR_DEVICE_LOST;
    }
}

SUBMIT_FAILED: // Skips waiting for destination queue to complete if it failed to submit
    foeGfxVkDestroyUploadRequest(pUploadContext->device, upload_request_from_handle(uploadRequest));

    if (stagingBuffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(pUploadContext->allocator, stagingBuffer, stagingAlloc);
    }

    if (result.value == FOE_SUCCESS && vkResult != VK_SUCCESS)
        result = vk_to_foeResult(vkResult);

    if (result.value == FOE_SUCCESS) {
        *pImage = image;
        *pAlloc = alloc;
        *pImageDepthView = depthView;
        *pImageStencilView = stencilView;
        *pSampler = sampler;
    } else {
        if (sampler != VK_NULL_HANDLE) {
            vkDestroySampler(pUploadContext->device, sampler, nullptr);
        }

        if (stencilView != VK_NULL_HANDLE) {
            vkDestroyImageView(pUploadContext->device, stencilView, nullptr);
        }

        if (depthView != VK_NULL_HANDLE) {
            vkDestroyImageView(pUploadContext->device, depthView, nullptr);
        }

        if (image != VK_NULL_HANDLE) {
            vmaDestroyImage(pUploadContext->allocator, image, alloc);
        }
    }

    return result;
}