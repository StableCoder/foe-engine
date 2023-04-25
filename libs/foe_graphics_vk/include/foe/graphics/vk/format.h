// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_FORMAT_H
#define FOE_GRAPHICS_VK_FORMAT_H

#include <foe/graphics/export.h>
#include <vulkan/vulkan.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Returns if the given format is a depth compatible format
FOE_GFX_EXPORT
bool foeGfxVkIsDepthFormat(VkFormat format);

/// Returns if the given format is a depth-stencil compatible format
FOE_GFX_EXPORT
bool foeGfxVkIsDepthStencilFormat(VkFormat format);

/** @brief Returns the number of bytes required to store each pixel
 * @param format Format type
 * @param aspects Aspects of the image to get the storage requirement for
 * @return Size in bytes per pixel required for storage.
 */
FOE_GFX_EXPORT
VkDeviceSize foeGfxVkBytesPerPixel(VkFormat format, VkImageAspectFlags aspects);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_VK_FORMAT_H