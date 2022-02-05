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

#include <foe/graphics/vk/render_graph/resource/image.hpp>

#include <vk_value_serialization.hpp>

#include "../../log.hpp"

#include <string>

VkAccessFlags foeGfxVkDetermineAccessFlags(VkImageLayout imageLayout) {
    switch (imageLayout) {
    case VK_IMAGE_LAYOUT_UNDEFINED:
        return 0;

    case VK_IMAGE_LAYOUT_GENERAL:
        return VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    // case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        return VK_ACCESS_SHADER_READ_BIT;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        return VK_ACCESS_TRANSFER_READ_BIT;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        return VK_ACCESS_TRANSFER_WRITE_BIT;

    case VK_IMAGE_LAYOUT_PREINITIALIZED:
        return VK_ACCESS_HOST_WRITE_BIT;

        // case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
        // case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
        // case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
        // case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
        // case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
        // case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:

    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
    case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
        return VK_ACCESS_TRANSFER_READ_BIT;

    default: {
        std::string serializedValue;
        vk_serialize("VkImageLayout", imageLayout, &serializedValue);
        FOE_LOG(foeVkGraphics, Fatal,
                "Attempted to get VkAccessFlags for unsupported VkImageLayout: {}",
                serializedValue);
        return 0;
    }
    }
}
