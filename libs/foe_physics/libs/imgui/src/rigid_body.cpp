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

#include "rigid_body.hpp"

#include <foe/ecs/id_to_string.hpp>
#include <foe/physics/component/rigid_body.hpp>
#include <imgui.h>

void imgui_foeRigidBody(foeRigidBody *pComponent) {
    ImGui::Separator();
    ImGui::Text("foeRigidBody");

    ImGui::InputFloat("Mass", &pComponent->mass);
    ImGui::Text("CollisionShape ID: %s", foeIdToString(pComponent->collisionShape).data());
}