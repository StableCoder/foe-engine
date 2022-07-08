// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "mesh.hpp"

#include <foe/graphics/resource/mesh.hpp>
#include <foe/graphics/resource/mesh_create_info.hpp>
#include <imgui.h>

void imgui_foeMesh(foeMesh const *pResource) {
    ImGui::Text("foeMesh");

    ImGui::Text("gfxData: %p", pResource->gfxData);
    ImGui::Text("gfxBones: %lu", pResource->gfxBones.size());
    ImGui::Text("gfxVertexComponent: %lu", pResource->gfxVertexComponent.size());
    // @todo Implement foeVertexComponent struct type ImGui display
    ImGui::Text("perVertexBoneWeights: %u", pResource->perVertexBoneWeights);
}

void imgui_foeMeshCreateInfo(foeMeshCreateInfo const *pCreateInfo) {
    ImGui::Text("foeMeshCreateInfo");

    if (pCreateInfo->source.get() != nullptr) {
        if (auto *ptr = dynamic_cast<foeMeshFileSource const *>(pCreateInfo->source.get()); ptr) {
            ImGui::Text("File Source:");
            ImGui::Text("File: %s", ptr->fileName.c_str());
            ImGui::Text("Mesh Name: %s", ptr->meshName.c_str());
            ImGui::Text("Post Processing Flags: %u", ptr->postProcessFlags);
        }
        if (auto *ptr = dynamic_cast<foeMeshCubeSource const *>(pCreateInfo->source.get()); ptr) {
            ImGui::Text("Cube Source:");
        }
        if (auto *ptr = dynamic_cast<foeMeshIcosphereSource const *>(pCreateInfo->source.get());
            ptr) {
            ImGui::Text("IcoSphere Source:");
            ImGui::Text("Recursion: %i", ptr->recursion);
        }
    } else {
        ImGui::Text("No Source??");
    }
}