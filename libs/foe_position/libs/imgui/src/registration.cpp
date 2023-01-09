// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/position/imgui/registration.hpp>

#include <foe/position/component/3d_pool.h>
#include <foe/position/type_defs.h>
#include <foe/simulation/imgui/registrar.hpp>
#include <foe/simulation/type_defs.h>

#include "position_3d.hpp"

namespace {

void imgui_foePositionComponents(foeEntityID entity, foeSimulation const *pSimulation) {
    // foePosition3d
    foePosition3dPool componentPool = (foePosition3dPool)foeSimulationGetComponentPool(
        pSimulation, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL);
    if (componentPool == FOE_NULL_HANDLE)
        return;

    foeEntityID const *const pStartID = foeEcsComponentPoolIdPtr(componentPool);
    foeEntityID const *const pEndID = pStartID + foeEcsComponentPoolSize(componentPool);

    foeEntityID const *pID = std::lower_bound(pStartID, pEndID, entity);

    if (pID != pEndID && *pID == entity) {
        foePosition3d **pComponentData =
            (foePosition3d **)foeEcsComponentPoolDataPtr(componentPool) + (pID - pStartID);

        imgui_foePosition3d(*pComponentData);
    }
}

} // namespace

foeResultSet foePositionImGuiRegister(foeSimulationImGuiRegistrar *pRegistrar) {
    return pRegistrar->registerElements(&imgui_foePositionComponents, nullptr, nullptr, nullptr);
}

foeResultSet foePositionImGuiDeregister(foeSimulationImGuiRegistrar *pRegistrar) {
    return pRegistrar->deregisterElements(&imgui_foePositionComponents, nullptr, nullptr, nullptr);
}