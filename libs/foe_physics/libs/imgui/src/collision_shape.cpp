// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "collision_shape.hpp"

#include <foe/physics/resource/collision_shape.hpp>
#include <foe/physics/resource/collision_shape_create_info.hpp>
#include <imgui.h>

void imgui_foeCollisionShape(foeCollisionShape const *pResource) {
    ImGui::Text("foeCollisionShape");

    ImGui::Text("CollisionShape %p", pResource->collisionShape.get());
}

void imgui_foeCollisionShapeCreateInfo(foeCollisionShapeCreateInfo const *pCreateInfo) {
    ImGui::Text("foeCollsionShapeCreateInfo");

    ImGui::Text("Box Size: X %.2f Y %.2f Z %.2f", pCreateInfo->boxSize.x, pCreateInfo->boxSize.y,
                pCreateInfo->boxSize.z);
}