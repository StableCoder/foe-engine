// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "bringup_registration.hpp"

#include <foe/simulation/imgui/registrar.hpp>
#include <foe/simulation/simulation.hpp>

#include "../simulation/armature_state.h"
#include "../simulation/armature_state_pool.h"
#include "../simulation/render_state_pool.h"
#include "armature.hpp"
#include "armature_state.hpp"
#include "render_state.hpp"

namespace {

void imgui_foeSkunkworksComponents(foeEntityID entity, foeSimulation const *pSimulation) {
    // foeArmatureState
    if (foeArmatureStatePool componentPool = (foeArmatureStatePool)foeSimulationGetComponentPool(
            pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_ARMATURE_STATE_POOL);
        componentPool != FOE_NULL_HANDLE) {
        foeEntityID const *const pStartID = foeEcsComponentPoolIdPtr(componentPool);
        foeEntityID const *const pEndID = pStartID + foeEcsComponentPoolSize(componentPool);

        foeEntityID const *pID = std::lower_bound(pStartID, pEndID, entity);

        if (pID != pEndID && *pID == entity) {
            size_t offset = pID - pStartID;
            foeArmatureState *pComponentData =
                (foeArmatureState *)foeEcsComponentPoolDataPtr(componentPool) + offset;
            imgui_foeArmatureState(pComponentData);
        }
    }

    // foeRenderState
    if (foeRenderStatePool componentPool = (foeRenderStatePool)foeSimulationGetComponentPool(
            pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_RENDER_STATE_POOL);
        componentPool != FOE_NULL_HANDLE) {
        foeEntityID const *const pStartID = foeEcsComponentPoolIdPtr(componentPool);
        foeEntityID const *const pEndID = pStartID + foeEcsComponentPoolSize(componentPool);

        foeEntityID const *pID = std::lower_bound(pStartID, pEndID, entity);

        if (pID != pEndID && *pID == entity) {
            size_t offset = pID - pStartID;
            foeRenderState *pComponentData =
                (foeRenderState *)foeEcsComponentPoolDataPtr(componentPool) + offset;

            imgui_foeRenderState(pComponentData);
        }
    }
}

void imgui_foeSkunkworksResources(foeResourceBase const *pResourceData) {
    if (pResourceData->rType == FOE_SKUNKWORKS_STRUCTURE_TYPE_ARMATURE)
        imgui_foeArmature((foeArmature const *)pResourceData);
}

void imgui_BringupResourceCreateInfo(foeResourceCreateInfo resourceCreateInfo) {
    if (foeResourceCreateInfoGetType(resourceCreateInfo) ==
        FOE_SKUNKWORKS_STRUCTURE_TYPE_ARMATURE_CREATE_INFO)
        imgui_foeArmatureCreateInfo(
            (foeArmatureCreateInfo const *)foeResourceCreateInfoGetData(resourceCreateInfo));
}

} // namespace

foeResultSet foeSkunkworksImGuiRegister(foeSimulationImGuiRegistrar *pRegistrar) {
    return pRegistrar->registerElements(&imgui_foeSkunkworksComponents,
                                        imgui_foeSkunkworksResources,
                                        imgui_BringupResourceCreateInfo, nullptr);
}

foeResultSet foeSkunkworksImGuiDeregister(foeSimulationImGuiRegistrar *pRegistrar) {
    return pRegistrar->deregisterElements(&imgui_foeSkunkworksComponents,
                                          imgui_foeSkunkworksResources,
                                          imgui_BringupResourceCreateInfo, nullptr);
}