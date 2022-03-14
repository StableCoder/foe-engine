/*
    Copyright (C) 2021-2022 George Cave.

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

#include "collision_shape.hpp"

#include <foe/physics/resource/collision_shape.hpp>
#include <foe/physics/resource/collision_shape_loader.hpp>
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