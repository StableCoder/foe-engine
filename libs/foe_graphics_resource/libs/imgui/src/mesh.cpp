// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "mesh.hpp"

#include <foe/external/imgui.h>
#include <foe/graphics/resource/mesh.hpp>
#include <foe/graphics/resource/mesh_create_info.h>

void imgui_foeMesh(foeMesh const *pResource) {
    ImGui::Text("foeMesh");

    ImGui::Text("gfxData: %p", pResource->gfxData);
    ImGui::Text("gfxBones: %lu", pResource->gfxBones.size());
    ImGui::Text("gfxVertexComponent: %lu", pResource->gfxVertexComponent.size());
    // @todo Implement foeVertexComponent struct type ImGui display
    ImGui::Text("perVertexBoneWeights: %u", pResource->perVertexBoneWeights);
}

void imgui_foeMeshFileCreateInfo(foeMeshFileCreateInfo const *pCreateInfo) {
    ImGui::Text("foeMeshFileCreateInfo");

    ImGui::Text("File: %s", pCreateInfo->pFile);
    ImGui::Text("Mesh Name: %s", pCreateInfo->pMesh);
    ImGui::Text("Post Processing Flags: %u", pCreateInfo->postProcessFlags);
}

void imgui_foeMeshCubeCreateInfo(foeMeshCubeCreateInfo const *pCreateInfo) {
    ImGui::Text("foeMeshCubeCreateInfo");
}

void imgui_foeMeshIcosphereCreateInfo(foeMeshIcosphereCreateInfo const *pCreateInfo) {
    ImGui::Text("foeMeshIcosphereCreateInfo");

    ImGui::Text("Recursion: %i", pCreateInfo->recursion);
}
