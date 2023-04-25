// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_IMGUI_VK_TYPE_HPP
#define FOE_GRAPHICS_VK_IMGUI_VK_TYPE_HPP

#include <foe/graphics/vk/imgui/export.h>
#include <vulkan/vulkan.h>

#include <string>

FOE_GFX_VK_IMGUI_EXPORT
void imgui_VkEnum32(std::string const &typeName, std::string const &label, uint32_t const &data);

FOE_GFX_VK_IMGUI_EXPORT
void imgui_VkEnum64(std::string const &typeName, std::string const &label, uint64_t const &data);

template <typename VkEnum>
void imgui_VkEnum(std::string const &typeName, std::string const &label, VkEnum const &data) {
    static_assert(sizeof(VkEnum) == 4 || sizeof(VkEnum) == 8,
                  "imgui_VkEnum only supports 32 and 64-bit types currently.");

    if constexpr (sizeof(VkEnum) == 4) {
        imgui_VkEnum32(typeName, label, data);
    } else {
        imgui_VkEnum64(typeName, label, data);
    }
}

FOE_GFX_VK_IMGUI_EXPORT
void imgui_VkBool32(std::string const &label, VkBool32 const &data);

#endif // FOE_GRAPHICS_VK_IMGUI_VK_TYPE_HPP