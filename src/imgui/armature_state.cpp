// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "armature_state.hpp"

#include <imgui.h>

#include "../simulation/armature_state.h"

void imgui_foeArmatureState(foeArmatureState *pArmatureState) {
    ImGui::Separator();
    ImGui::Text("foeArmatureState");

    ImGui::Text("Armature ID: 0x%08x", pArmatureState->armatureID);

    ImGui::Text("Animation ID: 0x%08x", pArmatureState->animationID);
    ImGui::Text("Animation TimePoint: %.2f", pArmatureState->time);
}