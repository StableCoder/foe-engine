// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_ERROR_COLOUR_IMAGE_H
#define FOE_GRAPHICS_ERROR_COLOUR_IMAGE_H

#include <foe/graphics/export.h>
#include <foe/graphics/upload_context.h>
#include <foe/result.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_GFX_EXPORT
foeResultSet foeCreateErrorColourImage(foeGfxUploadContext uploadContext,
                                       VkFormat format,
                                       uint32_t numMipLevels,
                                       uint32_t numCheckSquares,
                                       VmaAllocation *pAlloc,
                                       VkImage *pImage,
                                       VkImageView *pImageView,
                                       VkSampler *pSampler);

FOE_GFX_EXPORT
foeResultSet foeCreateErrorDepthStencilImage(foeGfxUploadContext uploadContext,
                                             uint32_t numMipLevels,
                                             uint32_t numCheckSquares,
                                             VmaAllocation *pAlloc,
                                             VkImage *pImage,
                                             VkImageView *pImageDepthView,
                                             VkImageView *pImageStencilView,
                                             VkSampler *pSampler);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_ERROR_COLOUR_IMAGE_H