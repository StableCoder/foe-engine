// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/imgui/fragment_descriptor.hpp>

#include <foe/graphics/vk/fragment_descriptor.h>
#include <foe/graphics/vk/imgui/vk_struct.hpp>
#include <imgui.h>

void imgui_foeGfxVkFragmentDescriptor(foeGfxVkFragmentDescriptor const &data) {
    ImGui::Text("Fragment Shader: %p", data.mFragment);

    if (data.pRasterizationSCI != nullptr) {
        if (ImGui::TreeNode("pRasterizationSCI")) {
            imgui_VkPipelineRasterizationStateCreateInfo(*data.pRasterizationSCI);
            ImGui::TreePop();
        }
    } else {
        ImGui::Text("pRasterizationSCI: N/A");
    }

    if (data.pDepthStencilSCI != nullptr) {
        if (ImGui::TreeNode("pDepthStencilSCI")) {
            imgui_VkPipelineDepthStencilStateCreateInfo(*data.pDepthStencilSCI);
            ImGui::TreePop();
        }
    } else {
        ImGui::Text("pDepthStencilSCI: N/A");
    }

    if (data.pColourBlendSCI != nullptr) {
        if (ImGui::TreeNode("pColourBlendSCI")) {
            imgui_VkPipelineColorBlendStateCreateInfo(*data.pColourBlendSCI);
            ImGui::TreePop();
        }
    } else {
        ImGui::Text("pColourBlendSCI: N/A");
    }
}