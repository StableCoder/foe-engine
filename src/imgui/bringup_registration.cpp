// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "bringup_registration.hpp"

#include <foe/resource/imgui/create_info.h>
#include <foe/resource/imgui/resource.h>
#include <foe/resource/pool.h>
#include <foe/simulation/imgui/registrar.hpp>
#include <foe/simulation/simulation.hpp>
#include <imgui.h>

#include "../simulation/armature_state.h"
#include "../simulation/armature_state_pool.h"
#include "../simulation/render_state_pool.h"
#include "armature.hpp"
#include "armature_state.hpp"
#include "render_state.hpp"

namespace {

void imgui_foeBringupComponents(foeEntityID entity, foeSimulation const *pSimulation) {
    // foeArmatureState
    if (foeArmatureStatePool componentPool = (foeArmatureStatePool)foeSimulationGetComponentPool(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL);
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
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL);
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

void imgui_foeBringupResources(
    foeResourceID resourceID,
    foeSimulation const *pSimulation,
    std::function<void(foeResourceCreateInfo)> showResourceCreateInfoDataFn) {
    foeResource resource = foeResourcePoolFind(pSimulation->resourcePool, resourceID);
    foeResourceDecrementRefCount(resource);

    if (resource == FOE_NULL_HANDLE)
        return;

    // foeArmature
    if (foeResourceGetType(resource) == FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE) {
        imgui_foeResource(resource);

        std::string resDataHeader = "Data: ";
        resDataHeader += foeResourceLoadStateToString(foeResourceGetState(resource));
        if (ImGui::CollapsingHeader(resDataHeader.c_str())) {
            if (foeResourceGetState(resource) == FOE_RESOURCE_LOAD_STATE_LOADED) {
                imgui_foeArmature((foeArmature const *)foeResourceGetData(resource));
            }
        }

        if (ImGui::CollapsingHeader("CreateInfo")) {
            foeResourceCreateInfo createInfo = FOE_NULL_HANDLE;
            foeResultSet result =
                foeSimulationGetResourceCreateInfo(pSimulation, resourceID, &createInfo);
            if (result.value != FOE_SUCCESS || createInfo == FOE_NULL_HANDLE)
                // @TODO - Implement proper error handling
                std::abort();

            if (createInfo != FOE_NULL_HANDLE) {
                imgui_foeResourceCreateInfo(createInfo);
                ImGui::Separator();
                showResourceCreateInfoDataFn(createInfo);

                foeResourceCreateInfoDecrementRefCount(createInfo);
            }
        }
    }
}

void imgui_BringupResourceCreateInfo(foeResourceCreateInfo resourceCreateInfo) {
    if (foeResourceCreateInfoGetType(resourceCreateInfo) ==
        FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_CREATE_INFO) {
        imgui_foeArmatureCreateInfo(
            (foeArmatureCreateInfo const *)foeResourceCreateInfoGetData(resourceCreateInfo));
    }
}

} // namespace

foeResultSet foeBringupImGuiRegister(foeSimulationImGuiRegistrar *pRegistrar) {
    return pRegistrar->registerElements(&imgui_foeBringupComponents, imgui_foeBringupResources,
                                        imgui_BringupResourceCreateInfo, nullptr);
}

foeResultSet foeBringupImGuiDeregister(foeSimulationImGuiRegistrar *pRegistrar) {
    return pRegistrar->deregisterElements(&imgui_foeBringupComponents, imgui_foeBringupResources,
                                          imgui_BringupResourceCreateInfo, nullptr);
}