// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "material.hpp"

#include <foe/ecs/id_to_string.hpp>
#include <foe/graphics/resource/material.hpp>
#include <foe/graphics/resource/material_create_info.hpp>
#include <foe/graphics/vk/imgui/fragment_descriptor.hpp>
#include <foe/graphics/vk/imgui/vk_struct.hpp>
#include <imgui.h>

void imgui_foeMaterial(foeMaterial const *pResource) {
    ImGui::Text("foeMaterial");

    foeResourceID id;
    if (pResource->fragmentShader != FOE_NULL_HANDLE)
        id = foeResourceGetID(pResource->fragmentShader);
    else
        id = FOE_INVALID_ID;
    ImGui::Text("fragmentShader: %u", id);

    if (pResource->image != FOE_NULL_HANDLE)
        id = foeResourceGetID(pResource->image);
    else
        id = FOE_INVALID_ID;
    ImGui::Text("image: %u", id);

    imgui_foeGfxVkFragmentDescriptor(*pResource->pGfxFragDescriptor);
}

void imgui_foeMaterialCreateInfo(foeMaterialCreateInfo const *pCreateInfo) {
    ImGui::Text("foeMaterialCreateInfo");

    ImGui::Text("Fragment Shader: %s", foeIdToString(pCreateInfo->fragmentShader).c_str());
    ImGui::Text("Image: %s", foeIdToString(pCreateInfo->image).c_str());

    if (pCreateInfo->hasRasterizationSCI) {
        if (ImGui::TreeNode("rasterizationSCI")) {
            imgui_VkPipelineRasterizationStateCreateInfo(pCreateInfo->rasterizationSCI);
            ImGui::TreePop();
        }
    } else {
        ImGui::Text("rasterizationSCI: No Data");
    }

    if (pCreateInfo->hasDepthStencilSCI) {
        if (ImGui::TreeNode("depthStencilSCI")) {
            imgui_VkPipelineDepthStencilStateCreateInfo(pCreateInfo->depthStencilSCI);
            ImGui::TreePop();
        }
    } else {
        ImGui::Text("depthStencilSCI: No Data");
    }

    if (pCreateInfo->hasColourBlendSCI) {
        if (ImGui::TreeNode("colourBlendSCI")) {
            imgui_VkPipelineColorBlendStateCreateInfo(pCreateInfo->colourBlendSCI);
            ImGui::TreePop();
        }
    } else {
        ImGui::Text("colourBlendSCI: No Data");
    }
}