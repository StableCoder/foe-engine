// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "bringup_registration.hpp"

#include <foe/resource/imgui/create_info.h>
#include <foe/resource/imgui/resource.h>
#include <foe/resource/pool.h>
#include <foe/simulation/imgui/registrar.hpp>
#include <foe/simulation/simulation.hpp>
#include <imgui.h>

#include "../simulation/armature_state_pool.hpp"
#include "../simulation/render_state_pool.hpp"
#include "armature.hpp"
#include "armature_state.hpp"
#include "render_state.hpp"

namespace {

void imgui_foeBringupComponents(foeEntityID entity, foeSimulation const *pSimulation) {
    // foeArmatureState

    if (auto *pPool = (foeArmatureStatePool *)foeSimulationGetComponentPool(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL);
        pPool) {
        auto offset = pPool->find(entity);
        if (offset != pPool->size()) {
            auto *pComponent = pPool->begin<1>() + offset;
            imgui_foeArmatureState(pComponent);
        }
    }

    // foeRenderState
    if (auto *pPool = (foeRenderStatePool *)foeSimulationGetComponentPool(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL);
        pPool) {
        auto offset = pPool->find(entity);
        if (offset != pPool->size()) {
            auto *pComponent = pPool->begin<1>() + offset;
            imgui_foeRenderState(pComponent);
        }
    }
}

void imgui_foeBringupResources(
    foeResourceID resourceID,
    foeSimulation const *pSimulation,
    std::function<void(foeResourceCreateInfo)> showResourceCreateInfoDataFn) {
    foeResource resource = foeResourcePoolFind(pSimulation->resourcePool, resourceID);

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