// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/physics/imgui/registration.hpp>

#include <foe/physics/component/rigid_body_pool.h>
#include <foe/physics/type_defs.h>
#include <foe/simulation/imgui/registrar.hpp>
#include <foe/simulation/simulation.hpp>

#include "collision_shape.hpp"
#include "rigid_body.hpp"

namespace {

void imgui_foePhysicsComponent(foeEntityID entity, foeSimulation const *pSimulation) {
    // foeRigidBody
    foeRigidBodyPool rigidBodyPool = (foeRigidBodyPool)foeSimulationGetComponentPool(
        pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL);

    if (rigidBodyPool != FOE_NULL_HANDLE) {
        foeEntityID const *const pStartID = foeEcsComponentPoolIdPtr(rigidBodyPool);
        foeEntityID const *const pEndID = pStartID + foeEcsComponentPoolSize(rigidBodyPool);

        foeEntityID const *pID = std::lower_bound(pStartID, pEndID, entity);

        if (pID != pEndID && *pID == entity) {
            size_t offset = pID - pStartID;

            foeRigidBody *pRigidBodyData =
                (foeRigidBody *)foeEcsComponentPoolDataPtr(rigidBodyPool) + offset;

            imgui_foeRigidBody(pRigidBodyData);
        }
    }
}

void imgui_foePhysicsResource(foeResourceBase const *pResourceData) {
    if (pResourceData->rType == FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE)
        imgui_foeCollisionShape((foeCollisionShape const *)pResourceData);
}

void imgui_foePhysicsResourceCreateInfo(foeResourceCreateInfo createInfo) {
    if (foeResourceCreateInfoGetType(createInfo) ==
        FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_CREATE_INFO)
        imgui_foeCollisionShapeCreateInfo(
            (foeCollisionShapeCreateInfo const *)foeResourceCreateInfoGetData(createInfo));
}

} // namespace

foeResultSet foePhysicsImGuiRegister(foeSimulationImGuiRegistrar *pRegistrar) {
    return pRegistrar->registerElements(imgui_foePhysicsComponent, imgui_foePhysicsResource,
                                        imgui_foePhysicsResourceCreateInfo, nullptr);
}

foeResultSet foePhysicsImGuiDeregister(foeSimulationImGuiRegistrar *pRegistrar) {
    return pRegistrar->deregisterElements(imgui_foePhysicsComponent, imgui_foePhysicsResource,
                                          imgui_foePhysicsResourceCreateInfo, nullptr);
}