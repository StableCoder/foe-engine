// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_IMAGE_HPP
#define FOE_GRAPHICS_IMAGE_HPP

#include <foe/graphics/export.h>
#include <foe/graphics/upload_buffer.h>
#include <foe/graphics/upload_context.h>
#include <foe/graphics/upload_request.h>
#include <foe/result.h>
#include <vulkan/vulkan.h>

/** @brief Returns the maximum mipmap levels for the given extent.
 * @param extent The extent dimensions.
 * @return The max possible mipmap levels.
 */
FOE_GFX_EXPORT uint32_t maxMipmapCount(VkExtent3D extent) noexcept;

/** @brief Returns the extent for a given mip level.
 * @param extent The original extent for mip level 0.
 * @param level The mip level to retrieve. Must be less than 32.
 * @return The extent for the given mip level.
 * @warning There is no safety for a mipmap level beyond the max, which would also return (1, 1).
 */
FOE_GFX_EXPORT VkExtent3D mipmapExtent(VkExtent3D extent, uint32_t mipLevel) noexcept;

/** @brief Returns the pixel count for given extent, over the number of given mipmap levels.
 * @param extent The extent of the base image.
 * @param levels The number of levels down to count the number of pixels for.
 * @return The overall pixel count.
 */
FOE_GFX_EXPORT VkDeviceSize pixelCount(VkExtent3D extent, uint32_t mipLevels) noexcept;

FOE_GFX_EXPORT foeResultSet
recordImageUploadCommands(foeGfxUploadContext uploadContext,
                          VkImageSubresourceRange const *pSubresourceRange,
                          uint32_t copyRegionCount,
                          VkBufferImageCopy const *pCopyRegions,
                          foeGfxUploadBuffer srcBuffer,
                          VkImage dstImage,
                          VkAccessFlags dstAccessFlags,
                          VkImageLayout dstImageLayout,
                          foeGfxUploadRequest *pUploadRequest);

FOE_GFX_EXPORT foeResultSet
recordImageUploadCommands(foeGfxUploadContext uploadContext,
                          VkImageSubresourceRange const *pSubresourceRange,
                          uint32_t copyRegionCount,
                          VkBufferImageCopy const *pCopyRegions,
                          VkBuffer srcBuffer,
                          VkImage dstImage,
                          VkAccessFlags dstAccessFlags,
                          VkImageLayout dstImageLayout,
                          foeGfxUploadRequest *pUploadRequest);

FOE_GFX_EXPORT VkImageAspectFlags formatAspects(VkFormat format) noexcept;

#endif // FOE_GRAPHICS_IMAGE_HPP