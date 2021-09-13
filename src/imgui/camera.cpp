/*
    Copyright (C) 2021 George Cave.

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

#include "camera.hpp"

#include <imgui.h>

#include "../camera.hpp"

void imgui_Camera(Camera *pCamera) {
    ImGui::Separator();
    ImGui::Text("Camera");

    ImGui::Text("View: %.1f x %.1f", pCamera->viewX, pCamera->viewY);

    ImGui::Text("Field of View Y: %.2f", pCamera->fieldOfViewY);

    ImGui::Text("Near Z: %.2f", pCamera->nearZ);
    ImGui::Text("Far Z: %.2f", pCamera->farZ);
}