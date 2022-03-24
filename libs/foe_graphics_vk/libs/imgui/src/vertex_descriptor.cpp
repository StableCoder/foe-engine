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

#include <foe/graphics/vk/imgui/vertex_descriptor.hpp>

#include <foe/graphics/vk/imgui/vk_struct.hpp>
#include <foe/graphics/vk/vertex_descriptor.hpp>
#include <imgui.h>

void imgui_foeGfxVertexDescriptor(foeGfxVertexDescriptor const &data) {
    ImGui::Text("Vertex Shader: %p", data.mVertex);
    ImGui::Text("Tessellation Control Shader: %p", data.mTessellationControl);
    ImGui::Text("Tessellation Evaluation Shader: %p", data.mTessellationEvaluation);
    ImGui::Text("Geometry Shader: %p", data.mGeometry);

    if (ImGui::TreeNode("Vertex Input Bindings")) {
        for (auto const &it : data.mVertexInputBindings) {
            imgui_VkVertexInputBindingDescription(it);
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Vertex Input Attributes")) {
        for (auto const &it : data.mVertexInputAttributes) {
            imgui_VkVertexInputAttributeDescription(it);
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Vertex Input State")) {
        imgui_VkPipelineVertexInputStateCreateInfo(data.mVertexInputSCI);
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Input Assembly State")) {
        imgui_VkPipelineInputAssemblyStateCreateInfo(data.mInputAssemblySCI);
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Tessellation State")) {
        imgui_VkPipelineTessellationStateCreateInfo(data.mTessellationSCI);
        ImGui::TreePop();
    }
}