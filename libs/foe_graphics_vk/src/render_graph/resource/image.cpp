// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

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

    case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
        return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

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
        FOE_LOG(foeVkGraphics, FOE_LOG_LEVEL_FATAL,
                "Attempted to get VkAccessFlags for unsupported VkImageLayout: {}",
                serializedValue);
        return 0;
    }
    }
}
