// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_ERROR_COLOUR_IMAGE_HPP
#define FOE_GRAPHICS_ERROR_COLOUR_IMAGE_HPP

#include <foe/error_code.h>
#include <foe/graphics/export.h>
#include <foe/graphics/upload_context.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

FOE_GFX_EXPORT foeResult foeCreateErrorColourImage(foeGfxUploadContext uploadContext,
                                                   VkFormat format,
                                                   uint32_t numMipLevels,
                                                   uint32_t numCheckSquares,
                                                   VmaAllocation *pAlloc,
                                                   VkImage *pImage,
                                                   VkImageView *pImageView,
                                                   VkSampler *pSampler);

FOE_GFX_EXPORT foeResult foeCreateErrorDepthStencilImage(foeGfxUploadContext uploadContext,
                                                         uint32_t numMipLevels,
                                                         uint32_t numCheckSquares,
                                                         VmaAllocation *pAlloc,
                                                         VkImage *pImage,
                                                         VkImageView *pImageDepthView,
                                                         VkImageView *pImageStencilView,
                                                         VkSampler *pSampler);

#endif // FOE_GRAPHICS_ERROR_COLOUR_IMAGE_HPP