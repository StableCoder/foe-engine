// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/position/imgui/registration.hpp>

#include <foe/position/component/3d_pool.hpp>
#include <foe/simulation/imgui/registrar.hpp>
#include <foe/simulation/type_defs.h>

#include "position_3d.hpp"

namespace {

void imgui_foePositionComponents(foeEntityID entity, foeSimulation const *pSimulation) {
    // foePosition3d
    auto *pPool = (foePosition3dPool *)foeSimulationGetComponentPool(
        pSimulation, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL);
    if (pPool != nullptr) {
        auto offset = pPool->find(entity);
        if (offset != pPool->size()) {
            auto *pComponent = pPool->begin<1>() + offset;

            imgui_foePosition3d(pComponent->get());
        }
    }
}

} // namespace

foeResultSet foePositionImGuiRegister(foeSimulationImGuiRegistrar *pRegistrar) {
    return pRegistrar->registerElements(&imgui_foePositionComponents, nullptr, nullptr, nullptr);
}

foeResultSet foePositionImGuiDeregister(foeSimulationImGuiRegistrar *pRegistrar) {
    return pRegistrar->deregisterElements(&imgui_foePositionComponents, nullptr, nullptr, nullptr);
}