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

#include "armature_state.hpp"

#include <imgui.h>

#include "../armature_state.hpp"

void imgui_foeArmatureState(foeArmatureState *pArmatureState) {
    ImGui::Separator();
    ImGui::Text("foeArmatureState");

    ImGui::Text("Armature ID: 0x%08x", pArmatureState->armatureID);

    ImGui::Text("Animation ID: 0x%08x", pArmatureState->animationID);
    ImGui::Text("Animation TimePoint: %.2f", pArmatureState->time);
}