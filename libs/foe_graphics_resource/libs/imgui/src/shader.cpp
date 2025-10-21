// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "shader.hpp"

#include <foe/external/imgui.h>
#include <foe/graphics/resource/shader.hpp>
#include <foe/graphics/resource/shader_create_info.h>
#include <foe/graphics/vk/imgui/shader.hpp>

void imgui_foeShader(foeShader const *pResource) {
    ImGui::Text("foeShader");

    ImGui::Text("Shader: %p", pResource->shader);
}

void imgui_foeShaderCreateInfo(foeShaderCreateInfo const *pCreateInfo) {
    ImGui::Text("foeShaderCreateInfo");

    ImGui::Text("File: %s", pCreateInfo->pFile);
    if (ImGui::TreeNode("gfxCreateInfo (foeGfxVkShaderCreateInfo)")) {
        imgui_foeGfxVkShaderCreateInfo(pCreateInfo->gfxCreateInfo);
        ImGui::TreePop();
    }
}