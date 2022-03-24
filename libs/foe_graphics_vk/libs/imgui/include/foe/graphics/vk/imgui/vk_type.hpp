/*
    Copyright (C) 2022 George Cave.

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

#ifndef FOE_GRAPHICS_VK_IMGUI_VK_TYPE_HPP
#define FOE_GRAPHICS_VK_IMGUI_VK_TYPE_HPP

#include <foe/graphics/vk/imgui/export.h>
#include <vulkan/vulkan.h>

#include <string>

FOE_GFX_VK_IMGUI_EXPORT void imgui_VkEnum32(std::string const &typeName,
                                            std::string const &label,
                                            VkFlags const &data);

FOE_GFX_VK_IMGUI_EXPORT void imgui_VkEnum64(std::string const &typeName,
                                            std::string const &label,
                                            VkFlags64 const &data);

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

FOE_GFX_VK_IMGUI_EXPORT void imgui_VkBool32(std::string const &label, VkBool32 const &data);

#endif // FOE_GRAPHICS_VK_IMGUI_VK_TYPE_HPP