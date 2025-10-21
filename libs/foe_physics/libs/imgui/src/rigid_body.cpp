// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "rigid_body.hpp"

#include <foe/ecs/id_to_string.hpp>
#include <foe/external/imgui.h>
#include <foe/physics/component/rigid_body.h>

void imgui_foeRigidBody(foeRigidBody *pComponent) {
    ImGui::Separator();
    ImGui::Text("foeRigidBody");

    ImGui::InputFloat("Mass", &pComponent->mass);
    ImGui::Text("CollisionShape ID: %s", foeIdToString(pComponent->collisionShape).data());
}