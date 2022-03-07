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

#include "yaml_export_registration.hpp"

#include <foe/imex/exporters.hpp>
#include <foe/imex/yaml/exporter.hpp>
#include <foe/simulation/simulation.hpp>

#include "error_code.hpp"
#include "simulation/armature_state_imex.hpp"
#include "simulation/armature_state_pool.hpp"
#include "simulation/camera_imex.hpp"
#include "simulation/camera_pool.hpp"
#include "simulation/render_state_imex.hpp"
#include "simulation/render_state_pool.hpp"

namespace {

std::vector<foeKeyYamlPair> exportComponents(foeEntityID entity,
                                             foeSimulationState const *pSimulationState) {
    std::vector<foeKeyYamlPair> keyDataPairs;

    // ArmatureState
    auto *pArmatureStatePool = (foeArmatureStatePool *)foeSimulationGetComponentPool(
        pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL);
    if (pArmatureStatePool) {
        if (auto searchIt = pArmatureStatePool->find(entity);
            searchIt != pArmatureStatePool->size()) {
            keyDataPairs.emplace_back(foeKeyYamlPair{
                .key = yaml_armature_state_key(),
                .data = yaml_write_ArmatureState(pArmatureStatePool->begin<1>()[searchIt]),
            });
        }
    }

    // Camera
    auto *pCameraPool = (foeCameraPool *)foeSimulationGetComponentPool(
        pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_POOL);
    if (pCameraPool) {
        if (auto searchIt = pCameraPool->find(entity); searchIt != pCameraPool->size()) {
            keyDataPairs.emplace_back(foeKeyYamlPair{
                .key = yaml_camera_key(),
                .data = yaml_write_Camera(*pCameraPool->begin<1>()[searchIt].get()),
            });
        }
    }

    // RenderState
    auto *pRenderStatePool = (foeRenderStatePool *)foeSimulationGetComponentPool(
        pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL);
    if (pRenderStatePool) {
        if (auto searchIt = pRenderStatePool->find(entity); searchIt != pRenderStatePool->size()) {
            keyDataPairs.emplace_back(foeKeyYamlPair{
                .key = yaml_render_state_key(),
                .data = yaml_write_RenderState(pRenderStatePool->begin<1>()[searchIt]),
            });
        }
    }

    return keyDataPairs;
}

void onDeregister(foeExporter exporter) {
    if (std::string_view{exporter.pName} == "Yaml") {
        // Component
        foeImexYamlDeregisterComponentFn(exportComponents);
    }
}

std::error_code onRegister(foeExporter exporter) {
    std::error_code errC;

    if (std::string_view{exporter.pName} == "Yaml") {
        // Component
        if (foeImexYamlRegisterComponentFn(exportComponents)) {
            errC = FOE_BRINGUP_ERROR_FAILED_TO_REGISTER_EXPORTERS;
            goto REGISTRATION_FAILED;
        }
    }

REGISTRATION_FAILED:
    if (errC)
        onDeregister(exporter);

    return errC;
}

} // namespace

auto foeBringupYamlRegisterExporters() -> std::error_code {
    return foeRegisterExportFunctionality(foeExportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}

void foeBringupYamlDeregisterExporters() {
    foeDeregisterExportFunctionality(foeExportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}