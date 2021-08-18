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

#ifndef FORMAT_HPP
#define FORMAT_HPP

#include <vulkan/vulkan.h>

#include <cstdlib>

constexpr bool isDepthFormat(VkFormat format) noexcept {
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

constexpr bool isDepthStencilFormat(VkFormat format) noexcept {
    switch (format) {
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return true;
    default:
        return false;
    }
}

constexpr VkDeviceSize bytesPerPixel(VkFormat format, VkImageAspectFlags aspects) noexcept {
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

#endif // FORMAT_HPP