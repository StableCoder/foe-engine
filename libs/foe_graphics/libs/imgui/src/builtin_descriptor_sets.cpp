// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/imgui/builtin_descriptor_sets.hpp>

#include <foe/external/imgui.h>

void imgui_foeBuiltinDescriptorSetLayoutFlags(std::string const &label,
                                              foeBuiltinDescriptorSetLayoutFlags const &data) {
    char const *pLayoutStr =
        builtin_set_layout_to_string((foeBuiltinDescriptorSetLayoutFlagBits)data);

    if (pLayoutStr != NULL)
        ImGui::Text("%s: %s", label.c_str(), pLayoutStr);
    else
        ImGui::Text("%s: N/A", label.c_str());
}