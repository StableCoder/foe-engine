// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "camera.hpp"

#include <imgui.h>

#include "../simulation/camera.hpp"

void imgui_Camera(Camera *pCamera) {
    ImGui::Separator();
    ImGui::Text("Camera");

    ImGui::Text("View: %.1f x %.1f", pCamera->viewX, pCamera->viewY);

    ImGui::Text("Field of View Y: %.2f", pCamera->fieldOfViewY);

    ImGui::Text("Near Z: %.2f", pCamera->nearZ);
    ImGui::Text("Far Z: %.2f", pCamera->farZ);
}