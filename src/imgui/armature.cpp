// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "armature.hpp"

#include <imgui.h>

#include "../simulation/armature.hpp"
#include "../simulation/armature_create_info.hpp"

void imgui_foeArmature(foeArmature const *pResource) {
    ImGui::Text("foeArmature");

    ImGui::Text("Armature Nodes: %lu", pResource->armature.size());

    // Animations List
    std::string animationsHeaderName =
        "Animations: " + std::to_string(pResource->animations.size());
    if (ImGui::TreeNode(animationsHeaderName.c_str())) {
        for (auto const &animation : pResource->animations) {
            if (ImGui::TreeNode(animation.name.c_str())) {
                ImGui::Text("Duration: %.2f", animation.duration);
                ImGui::Text("Ticks Per Second: %.2f", animation.ticksPerSecond);
                ImGui::Text("Animation Channels: %lu", animation.nodeChannels.size());

                ImGui::TreePop();
            }
        }

        ImGui::TreePop();
    }
}

void imgui_foeArmatureCreateInfo(foeArmatureCreateInfo const *pCreateInfo) {
    ImGui::Text("foeArmatureCreateInfo");

    ImGui::Text("File: %s", pCreateInfo->fileName.c_str());
    ImGui::Text("Root Node: %s", pCreateInfo->rootArmatureNode.c_str());

    std::string animationsHeaderName = "Animations: " + std::to_string(pCreateInfo->animationCount);
    if (ImGui::TreeNode(animationsHeaderName.c_str())) {
        for (uint32_t i = 0; i < pCreateInfo->animationCount; ++i) {
            auto const &animation = pCreateInfo->pAnimations[i];
            ImGui::Text("File: %s", animation.file.c_str());
            ImGui::Text("Animation Name: %s", animation.animationName.c_str());
        }

        ImGui::TreePop();
    }
}