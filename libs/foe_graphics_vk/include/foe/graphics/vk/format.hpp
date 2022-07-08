// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_FORMAT_HPP
#define FOE_GRAPHICS_VK_FORMAT_HPP

#include <vulkan/vulkan.h>

#include <cstdlib>

/// Returns if the given format is a depth compatible format
constexpr bool foeGfxVkIsDepthFormat(VkFormat format) noexcept {
    switch (format) {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return true;
    default:
        return false;
    }
}

/// Returns if the given format is a depth-stencil compatible format
constexpr bool foeGfxVkIsDepthStencilFormat(VkFormat format) noexcept {
    switch (format) {
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return true;
    default:
        return false;
    }
}

/** @brief Returns the number of bytes required to store each pixel
 * @param format Format type
 * @param aspects Aspects of the image to get the storage requirement for
 * @return Size in bytes per pixel required for storage.
 */
constexpr VkDeviceSize foeGfxVkBytesPerPixel(VkFormat format, VkImageAspectFlags aspects) noexcept {
    VkDeviceSize bpp = 0;

    if ((aspects & VK_IMAGE_ASPECT_COLOR_BIT) == VK_IMAGE_ASPECT_COLOR_BIT) {
        switch (format) {
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_UNORM:
            bpp += 4;
            break;

        case VK_FORMAT_R8G8B8_UNORM:
        case VK_FORMAT_B8G8R8_UNORM:
            bpp += 3;
            break;

        case VK_FORMAT_R8_UNORM:
            bpp += 1;
            break;

        default:
            std::abort();
        }
    }

    if ((aspects & VK_IMAGE_ASPECT_DEPTH_BIT) == VK_IMAGE_ASPECT_DEPTH_BIT) {
        switch (format) {
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
        case VK_FORMAT_D32_SFLOAT:
            bpp += 4;
            break;

        case VK_FORMAT_D24_UNORM_S8_UINT:
            bpp += 3;
            break;

        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D16_UNORM:
            bpp += 2;

        default:
            std::abort();
        }
    }

    if ((aspects & VK_IMAGE_ASPECT_STENCIL_BIT) == VK_IMAGE_ASPECT_STENCIL_BIT) {
        switch (format) {
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D16_UNORM_S8_UINT:
            bpp += 1;
            break;

        default:
            std::abort();
        }
    }

    return bpp;
}

#endif // FOE_GRAPHICS_VK_FORMAT_HPP