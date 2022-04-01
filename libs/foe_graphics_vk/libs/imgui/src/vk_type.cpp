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

#include <foe/graphics/vk/imgui/vk_type.hpp>

#include <fmt/core.h>
#include <imgui.h>
#include <vk_value_serialization.hpp>

void imgui_VkEnum32(std::string const &typeName, std::string const &label, uint32_t const &data) {
    std::string serializedStr;
    assert(vk_serialize(typeName.c_str(), data, &serializedStr) ==
           STEC_VK_SERIALIZATION_RESULT_SUCCESS);
    ImGui::Text("%s: %s", label.c_str(), serializedStr.c_str());
}

void imgui_VkEnum64(std::string const &typeName, std::string const &label, uint64_t const &data) {
    std::string serializedStr;
    assert(vk_serialize(typeName.c_str(), data, &serializedStr) ==
           STEC_VK_SERIALIZATION_RESULT_SUCCESS);
    ImGui::Text("%s: %s", label.c_str(), serializedStr.c_str());
}

void imgui_VkBool32(std::string const &label, VkBool32 const &data) {
    std::string fmtStr = fmt::format("{}", (bool)data);
    ImGui::Text("%s: %s", label.c_str(), fmtStr.c_str());
}