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

#include "bringup_registration.hpp"

#include <foe/resource/imgui/create_info.h>
#include <foe/resource/imgui/resource.h>
#include <foe/resource/pool.h>
#include <foe/simulation/imgui/registrar.hpp>
#include <foe/simulation/simulation.hpp>
#include <imgui.h>

#include "../simulation/armature_state_pool.hpp"
#include "../simulation/camera_pool.hpp"
#include "../simulation/render_state_pool.hpp"
#include "armature.hpp"
#include "armature_state.hpp"
#include "camera.hpp"
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

    // Camera
    if (auto *pPool = (foeCameraPool *)foeSimulationGetComponentPool(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_POOL);
        pPool) {
        auto offset = pPool->find(entity);
        if (offset != pPool->size()) {
            auto *pComponent = pPool->begin<1>() + offset;
            auto *pCamera = dynamic_cast<Camera *>(pComponent->get());

            if (pCamera)
                imgui_Camera(pCamera);
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
            if (foeResourceGetState(resource) == foeResourceLoadState::Loaded) {
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

auto foeBringupImGuiRegister(foeSimulationImGuiRegistrar *pRegistrar) -> std::error_code {
    return pRegistrar->registerElements(&imgui_foeBringupComponents, imgui_foeBringupResources,
                                        imgui_BringupResourceCreateInfo, nullptr);
}

auto foeBringupImGuiDeregister(foeSimulationImGuiRegistrar *pRegistrar) -> std::error_code {
    return pRegistrar->deregisterElements(&imgui_foeBringupComponents, imgui_foeBringupResources,
                                          imgui_BringupResourceCreateInfo, nullptr);
}