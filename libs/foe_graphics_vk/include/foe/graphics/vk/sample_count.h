// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_SAMPLE_COUNT_H
#define FOE_GRAPHICS_VK_SAMPLE_COUNT_H

#include <foe/graphics/export.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Matches the given integer to an appropriate flag
 * @param sampleCount Count to try to match
 * @return The appropriate flag to match the parameter. If there is no match to map, returns 0.
 */
FOE_GFX_EXPORT VkSampleCountFlags foeGfxVkGetSampleCountFlags(int sampleCount);

/** @brief Converts the given flag to an integer value
 * @param flags Flag to convert
 * @return The appropriate sample count as a number. 0 if it's not a valid flag value.
 */
FOE_GFX_EXPORT int foeGfxVkGetSampleCount(VkSampleCountFlags flags);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_VK_SAMPLE_COUNT_H