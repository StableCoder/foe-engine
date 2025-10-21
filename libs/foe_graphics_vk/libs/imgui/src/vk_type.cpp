// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/imgui/vk_type.hpp>

#include <fmt/core.h>
#include <foe/external/imgui.h>
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