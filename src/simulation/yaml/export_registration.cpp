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

#include "export_registration.hpp"

#include <foe/imex/exporters.hpp>
#include <foe/imex/yaml/exporter.hpp>

#include "../armature.hpp"
#include "../armature_loader.hpp"
#include "../armature_pool.hpp"
#include "../armature_state_imex.hpp"
#include "../armature_state_pool.hpp"
#include "../camera_imex.hpp"
#include "../camera_pool.hpp"
#include "../render_state_imex.hpp"
#include "../render_state_pool.hpp"
#include "../type_defs.h"
#include "armature.hpp"
#include "error_code.hpp"

namespace {

std::vector<foeKeyYamlPair> exportResources(foeResourceID resource,
                                            foeSimulation const *pSimulation) {
    std::vector<foeKeyYamlPair> keyDataPairs;

    // Armature
    auto *pArmaturePool = (foeArmaturePool *)foeSimulationGetResourcePool(
        pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_POOL);

    if (pArmaturePool != nullptr) {
        foeResource armature = pArmaturePool->find(resource);

        if (armature != FOE_NULL_HANDLE) {
            auto createInfo = foeResourceGetCreateInfo(armature);

            if (foeResourceCreateInfoGetType(createInfo) ==
                FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_CREATE_INFO) {
                auto const *pCreateInfo =
                    (foeArmatureCreateInfo const *)foeResourceCreateInfoGetData(createInfo);

                keyDataPairs.emplace_back(foeKeyYamlPair{
                    .key = yaml_armature_key(),
                    .data = yaml_write_armature(*pCreateInfo),
                });
            }
        }
    }

    return keyDataPairs;
}

std::vector<foeKeyYamlPair> exportComponents(foeEntityID entity, foeSimulation const *pSimulation) {
    std::vector<foeKeyYamlPair> keyDataPairs;

    // ArmatureState
    auto *pArmatureStatePool = (foeArmatureStatePool *)foeSimulationGetComponentPool(
        pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL);
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
        pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_POOL);
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
        pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL);
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

        // Resource
        foeImexYamlDeregisterResourceFn(exportResources);
    }
}

std::error_code onRegister(foeExporter exporter) {
    std::error_code errC;

    if (std::string_view{exporter.pName} == "Yaml") {
        // Resource
        if (foeImexYamlRegisterResourceFn(exportResources)) {
            errC = FOE_BRINGUP_YAML_ERROR_FAILED_TO_REGISTER_RESOURCE_EXPORTERS;
            goto REGISTRATION_FAILED;
        }

        // Component
        if (foeImexYamlRegisterComponentFn(exportComponents)) {
            errC = FOE_BRINGUP_YAML_ERROR_FAILED_TO_REGISTER_COMPONENT_EXPORTERS;
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