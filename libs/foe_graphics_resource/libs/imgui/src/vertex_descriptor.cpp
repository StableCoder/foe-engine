/*
    Copyright (C) 2021-2022 George Cave.

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

#include "vertex_descriptor.hpp"

#include <foe/graphics/resource/vertex_descriptor.hpp>
#include <foe/graphics/resource/vertex_descriptor_create_info.hpp>
#include <foe/graphics/vk/imgui/vertex_descriptor.hpp>
#include <foe/graphics/vk/imgui/vk_struct.hpp>
#include <imgui.h>

void imgui_foeVertexDescriptor(foeVertexDescriptor const *pResource) {
    ImGui::Text("foeVertexDescriptor");

    ImGui::Text("Vertex Shader: %p", pResource->vertexShader);
    ImGui::Text("Tessellation Control Shader: %p", pResource->tessellationControlShader);
    ImGui::Text("Tessellation Evaluation Shader: %p", pResource->tessellationEvaluationShader);
    ImGui::Text("Geometry Shader: %p", pResource->geometryShader);

    imgui_foeGfxVertexDescriptor(pResource->vertexDescriptor);
}

void imgui_foeVertexDescriptorCreateInfo(foeVertexDescriptorCreateInfo const *pCreateInfo) {
    ImGui::Text("foeVertexDescriptorCreateInfo");

    ImGui::Text("Vertex Shader: %u", pCreateInfo->vertexShader);
    ImGui::Text("Tessellation Control Shader: %u", pCreateInfo->tessellationControlShader);
    ImGui::Text("Tessellation Evaluation Shader: %u", pCreateInfo->tessellationEvaluationShader);
    ImGui::Text("Geometry Shader: %u", pCreateInfo->geometryShader);
}
