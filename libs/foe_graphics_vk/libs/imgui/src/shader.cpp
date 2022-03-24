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

#include <foe/graphics/vk/imgui/shader.hpp>

#include <foe/graphics/imgui/builtin_descriptor_sets.hpp>
#include <foe/graphics/vk/imgui/vk_struct.hpp>
#include <foe/graphics/vk/shader.hpp>
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