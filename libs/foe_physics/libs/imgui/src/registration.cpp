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
#include <foe/physics/type_defs.h>
#include <foe/resource/imgui/create_info.h>
#include <foe/resource/imgui/resource.h>
#include <foe/resource/pool.h>
#include <foe/resource/resource.h>
#include <foe/simulation/imgui/registrar.hpp>
#include <foe/simulation/simulation.hpp>
#include <imgui.h>

#include "collision_shape.hpp"
#include "rigid_body.hpp"

namespace {

void imgui_foePhysicsComponent(foeEntityID entity, foeSimulation const *pSimulation) {
    // foeRigidBody
    auto *pRigidBodyPool = (foeRigidBodyPool *)foeSimulationGetComponentPool(
        pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL);

    if (pRigidBodyPool != nullptr) {
        auto offset = pRigidBodyPool->find(entity);
        if (offset != pRigidBodyPool->size()) {
            auto *pComponent = pRigidBodyPool->begin<1>() + offset;
            imgui_foeRigidBody(pComponent);
        }
    }
}

void imgui_foePhysicsResource(
    foeResourceID resourceID,
    foeSimulation const *pSimulation,
    std::function<void(foeResourceCreateInfo)> showResourceCreateInfoDataFn) {
    foeResource resource = foeResourcePoolFind(pSimulation->resourcePool, resourceID);

    if (resource == FOE_NULL_HANDLE)
        return;

    // foeCollisionShape
    if (foeResourceGetType(resource) == FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE) {
        imgui_foeResource(resource);

        std::string resDataHeader = "Data: ";
        resDataHeader += foeResourceLoadStateToString(foeResourceGetState(resource));
        if (ImGui::CollapsingHeader(resDataHeader.c_str())) {
            if (foeResourceGetState(resource) == foeResourceLoadState::Loaded) {
                imgui_foeCollisionShape((foeCollisionShape const *)foeResourceGetData(resource));
            }
        }

        if (ImGui::CollapsingHeader("CreateInfo")) {
            foeResourceCreateInfo createInfo = foeResourceGetCreateInfo(resource);
            if (createInfo != FOE_NULL_HANDLE) {
                imgui_foeResourceCreateInfo(createInfo);
                ImGui::Separator();
                showResourceCreateInfoDataFn(createInfo);

                foeResourceCreateInfoDecrementRefCount(createInfo);
            }
        }
    }
}

void imgui_foePhysicsResourceCreateInfo(foeResourceCreateInfo createInfo) {
    if (foeResourceCreateInfoGetType(createInfo) ==
        FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_CREATE_INFO) {
        imgui_foeCollisionShapeCreateInfo(
            (foeCollisionShapeCreateInfo const *)foeResourceCreateInfoGetData(createInfo));
    }
}

} // namespace

auto foePhysicsImGuiRegister(foeSimulationImGuiRegistrar *pRegistrar) -> std::error_code {
    return pRegistrar->registerElements(imgui_foePhysicsComponent, imgui_foePhysicsResource,
                                        imgui_foePhysicsResourceCreateInfo, nullptr);
}

auto foePhysicsImGuiDeregister(foeSimulationImGuiRegistrar *pRegistrar) -> std::error_code {
    return pRegistrar->deregisterElements(imgui_foePhysicsComponent, imgui_foePhysicsResource,
                                          imgui_foePhysicsResourceCreateInfo, nullptr);
}