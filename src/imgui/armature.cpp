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

    std::string animationsHeaderName =
        "Animations: " + std::to_string(pCreateInfo->animations.size());
    if (ImGui::TreeNode(animationsHeaderName.c_str())) {
        for (auto const &animation : pCreateInfo->animations) {
            std::string fileNodeName = "File: " + animation.file;
            if (ImGui::TreeNode(fileNodeName.c_str())) {
                ImGui::Text("Animation Names:");
                for (auto const &it : animation.animationNames) {
                    ImGui::Text("- %s", it.c_str());
                }

                ImGui::TreePop();
            }
        }

        ImGui::TreePop();
    }
}