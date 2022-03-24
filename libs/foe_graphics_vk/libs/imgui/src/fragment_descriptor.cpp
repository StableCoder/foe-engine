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