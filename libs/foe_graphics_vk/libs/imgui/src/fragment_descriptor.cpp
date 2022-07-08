// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/imgui/fragment_descriptor.hpp>

#include <foe/graphics/vk/fragment_descriptor.hpp>
#include <foe/graphics/vk/imgui/vk_struct.hpp>
#include <imgui.h>

void imgui_foeGfxVkFragmentDescriptor(foeGfxVkFragmentDescriptor const &data) {
    ImGui::Text("Fragment Shader: %p", data.mFragment);

    if (data.hasRasterizationSCI) {
        if (ImGui::TreeNode("mRasterizationSCI")) {
            imgui_VkPipelineRasterizationStateCreateInfo(data.mRasterizationSCI);
            ImGui::TreePop();
        }
    } else {
        ImGui::Text("mRasterizationSCI: No");
    }

    if (data.hasDepthStencilSCI) {
        if (ImGui::TreeNode("mDepthStencilSCI")) {
            imgui_VkPipelineDepthStencilStateCreateInfo(data.mDepthStencilSCI);
            ImGui::TreePop();
        }
    } else {
        ImGui::Text("mDepthStencilSCI: No");
    }

    if (data.hasColourBlendSCI) {
        if (ImGui::TreeNode("mColourBlendSCI")) {
            imgui_VkPipelineColorBlendStateCreateInfo(data.mColourBlendSCI);
            ImGui::TreePop();
        }
    } else {
        ImGui::Text("mColourBlendSCI: No");
    }
}