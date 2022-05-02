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

#include "shader.hpp"

#include <foe/graphics/resource/shader.hpp>
#include <foe/graphics/resource/shader_create_info.hpp>
#include <foe/graphics/vk/imgui/shader.hpp>
#include <imgui.h>

void imgui_foeShader(foeShader const *pResource) {
    ImGui::Text("foeShader");

    ImGui::Text("Shader: %p", pResource->shader);
}

void imgui_foeShaderCreateInfo(foeShaderCreateInfo const *pCreateInfo) {
    ImGui::Text("foeShaderCreateInfo");

    ImGui::Text("File: %s", pCreateInfo->shaderCodeFile.c_str());
    if (ImGui::TreeNode("gfxCreateInfo (foeGfxVkShaderCreateInfo)")) {
        imgui_foeGfxVkShaderCreateInfo(pCreateInfo->gfxCreateInfo);
        ImGui::TreePop();
    }
}