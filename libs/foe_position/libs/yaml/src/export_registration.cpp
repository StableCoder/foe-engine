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

#include <foe/position/yaml/export_registration.hpp>

#include <foe/imex/exporters.hpp>
#include <foe/imex/yaml/exporter.hpp>
#include <foe/position/component/3d_pool.hpp>
#include <foe/simulation/simulation.hpp>

#include "3d.hpp"
#include "error_code.hpp"

namespace {

std::vector<foeKeyYamlPair> exportComponents(foeEntityID entity,
                                             foeSimulationState const *pSimulationState) {
    std::vector<foeKeyYamlPair> keyDataPairs;

    auto *pPosition3dPool = (foePosition3dPool *)foeSimulationGetComponentPool(
        pSimulationState, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL);
    if (pPosition3dPool != nullptr) {
        if (auto searchIt = pPosition3dPool->find(entity); searchIt != pPosition3dPool->size()) {
            keyDataPairs.emplace_back(foeKeyYamlPair{
                .key = yaml_position3d_key(),
                .data = yaml_write_position3d(*pPosition3dPool->begin<1>()[searchIt].get()),
            });
        }
    }

    return keyDataPairs;
}

void onDeregister(foeExporter exporter) {
    if (std::string_view{exporter.pName} == "Yaml") {
        // Components
        foeImexYamlDeregisterComponentFn(exportComponents);
    }
}

std::error_code onRegister(foeExporter exporter) {
    std::error_code errC;

    if (std::string_view{exporter.pName} == "Yaml") {
        // Components
        if (foeImexYamlRegisterComponentFn(exportComponents)) {
            errC = FOE_POSITION_YAML_ERROR_FAILED_TO_REGISTER_3D_EXPORTER;
            goto REGISTRATION_FAILED;
        }
    }

REGISTRATION_FAILED:
    if (errC)
        onDeregister(exporter);

    return errC;
}

} // namespace

auto foePositionYamlRegisterExporters() -> std::error_code {
    return foeRegisterExportFunctionality(foeExportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}

void foePositionYamlDeregisterExporters() {
    foeDeregisterExportFunctionality(foeExportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}