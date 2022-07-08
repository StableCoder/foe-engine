// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/imgui/builtin_descriptor_sets.hpp>

#include <imgui.h>

void imgui_foeBuiltinDescriptorSetLayoutFlags(std::string const &label,
                                              foeBuiltinDescriptorSetLayoutFlags const &data) {
    std::string serializedStr =
        builtin_set_layout_to_string((foeBuiltinDescriptorSetLayoutFlagBits)data);
    ImGui::Text("%s: %s", label.c_str(), serializedStr.c_str());
}