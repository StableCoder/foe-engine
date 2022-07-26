// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

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
        for (uint32_t i = 0; i < data.vertexInputBindingCount; ++i) {
            imgui_VkVertexInputBindingDescription(data.pVertexInputBindings[i]);
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Vertex Input Attributes")) {
        for (uint32_t i = 0; i < data.vertexInputAttributeCount; ++i) {
            imgui_VkVertexInputAttributeDescription(data.pVertexInputAttributes[i]);
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