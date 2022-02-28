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

#include <foe/physics/imgui/registration.hpp>

#include <foe/physics/component/rigid_body_pool.hpp>
#include <foe/physics/resource/collision_shape_pool.hpp>
#include <foe/physics/type_defs.h>
#include <foe/simulation/imgui/registrar.hpp>

#include "collision_shape.hpp"
#include "rigid_body.hpp"

namespace {

void imgui_foePhysicsComponents(foeEntityID entity, foeSimulationState const *pSimulationState) {
    // foeRigidBody
    auto *pRigidBodyPool = (foeRigidBodyPool *)foeSimulationGetComponentPool(
        pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL);

    if (pRigidBodyPool != nullptr) {
        auto offset = pRigidBodyPool->find(entity);
        if (offset != pRigidBodyPool->size()) {
            auto *pComponent = pRigidBodyPool->begin<1>() + offset;
            imgui_render_foeRigidBody(pComponent);
        }
    }
}

void imgui_foePhysicsResources(foeResourceID resource, foeSimulationState const *pSimulationState) {
    // foeCollisionShape
    auto *pCollisionShapePool = (foeCollisionShapePool *)foeSimulationGetResourcePool(
        pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_POOL);

    if (pCollisionShapePool != nullptr) {
        auto pResource = pCollisionShapePool->find(resource);
        if (pResource != nullptr) {
            imgui_render_foeCollisionShape(pResource);
        }
    }
}

} // namespace

auto foePhysicsImGuiRegister(foeSimulationImGuiRegistrar *pRegistrar) -> std::error_code {
    return pRegistrar->registerElements(&imgui_foePhysicsComponents, &imgui_foePhysicsResources,
                                        nullptr);
}

auto foePhysicsImGuiDeregister(foeSimulationImGuiRegistrar *pRegistrar) -> std::error_code {
    return pRegistrar->deregisterElements(&imgui_foePhysicsComponents, &imgui_foePhysicsResources,
                                          nullptr);
}