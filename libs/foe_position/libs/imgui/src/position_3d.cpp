// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "position_3d.hpp"

#include <foe/position/component/3d.hpp>
#include <imgui.h>

void imgui_foePosition3d(foePosition3d *pPosition3d) {
    ImGui::Separator();
    ImGui::Text("foePosition3d");

    ImGui::InputFloat3("Position", reinterpret_cast<float *>(&pPosition3d->position));
    ImGui::InputFloat4("Orientation(Quat)", reinterpret_cast<float *>(&pPosition3d->orientation));
}