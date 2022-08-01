// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/imgui/shader.hpp>

#include <foe/graphics/imgui/builtin_descriptor_sets.hpp>
#include <foe/graphics/vk/imgui/vk_struct.hpp>
#include <foe/graphics/vk/shader.h>
#include <imgui.h>

void imgui_foeGfxVkShaderCreateInfo(foeGfxVkShaderCreateInfo const &data) {
    imgui_foeBuiltinDescriptorSetLayoutFlags(
        "builtinSetLayouts (foeBuiltinDescriptorSetLayoutFlags)", data.builtinSetLayouts);

    if (ImGui::TreeNode("descriptorSetLayoutCI (VkDescriptorSetLayoutCreateInfo)")) {
        imgui_VkDescriptorSetLayoutCreateInfo(data.descriptorSetLayoutCI);
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("pushConstantRange (VkPushConstantRange)")) {
        imgui_VkPushConstantRange(data.pushConstantRange);
        ImGui::TreePop();
    }
}