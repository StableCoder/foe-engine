/*
    Copyright (C) 2021 George Cave.

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

#ifndef FOE_GRAPHICS_VK_SAMPLE_COUNT_HPP
#define FOE_GRAPHICS_VK_SAMPLE_COUNT_HPP

#include <vulkan/vulkan.h>

/** @brief Matches the given integer to an appropriate flag
 * @param sampleCount Count to try to match
 * @return The appropriate flag to match the parameter. If there is no match to map, returns 0.
 */
constexpr auto foeGfxVkGetSampleCountFlags(int sampleCount) -> VkSampleCountFlags {
    switch (sampleCount) {
    case 1:
        return VK_SAMPLE_COUNT_1_BIT;

    case 2:
        return VK_SAMPLE_COUNT_2_BIT;

    case 4:
        return VK_SAMPLE_COUNT_4_BIT;

    case 8:
        return VK_SAMPLE_COUNT_8_BIT;

    case 16:
        return VK_SAMPLE_COUNT_16_BIT;

    case 32:
        return VK_SAMPLE_COUNT_32_BIT;

    case 64:
        return VK_SAMPLE_COUNT_64_BIT;

    default:
        return static_cast<VkSampleCountFlags>(0);
    }
}

/** @brief Converts the given flag to an integer value
 * @param flags Flag to convert
 * @return The appropriate sample count as a number. 0 if it's not a valid flag value.
 */
constexpr int foeGfxVkGetSampleCount(VkSampleCountFlags flags) {
    switch (flags) {
    case VK_SAMPLE_COUNT_1_BIT:
        return 1;

    case VK_SAMPLE_COUNT_2_BIT:
        return 2;

    case VK_SAMPLE_COUNT_4_BIT:
        return 4;

    case VK_SAMPLE_COUNT_8_BIT:
        return 8;

    case VK_SAMPLE_COUNT_16_BIT:
        return 16;

    case VK_SAMPLE_COUNT_32_BIT:
        return 32;

    case VK_SAMPLE_COUNT_64_BIT:
        return 64;

    default:
        return 0;
    }
}

#endif // FOE_GRAPHICS_VK_SAMPLE_COUNT_HPP