// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "vertex_descriptor.hpp"

#include <foe/external/imgui.h>
#include <foe/graphics/resource/vertex_descriptor.hpp>
#include <foe/graphics/resource/vertex_descriptor_create_info.h>
#include <foe/graphics/vk/imgui/vertex_descriptor.hpp>
#include <foe/graphics/vk/imgui/vk_struct.hpp>

void imgui_foeVertexDescriptor(foeVertexDescriptor const *pResource) {
    ImGui::Text("foeVertexDescriptor");

    ImGui::Text("Vertex Shader: %p", pResource->vertexShader);
    ImGui::Text("Tessellation Control Shader: %p", pResource->tessellationControlShader);
    ImGui::Text("Tessellation Evaluation Shader: %p", pResource->tessellationEvaluationShader);
    ImGui::Text("Geometry Shader: %p", pResource->geometryShader);

    imgui_foeGfxVkVertexDescriptor(pResource->vertexDescriptor);
}

void imgui_foeVertexDescriptorCreateInfo(foeVertexDescriptorCreateInfo const *pCreateInfo) {
    ImGui::Text("foeVertexDescriptorCreateInfo");

    ImGui::Text("Vertex Shader: %u", pCreateInfo->vertexShader);
    ImGui::Text("Tessellation Control Shader: %u", pCreateInfo->tessellationControlShader);
    ImGui::Text("Tessellation Evaluation Shader: %u", pCreateInfo->tessellationEvaluationShader);
    ImGui::Text("Geometry Shader: %u", pCreateInfo->geometryShader);
}
