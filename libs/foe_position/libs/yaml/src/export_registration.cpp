// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/position/yaml/export_registration.h>

#include <foe/imex/exporters.h>
#include <foe/imex/yaml/exporter.hpp>
#include <foe/position/component/3d_pool.hpp>
#include <foe/simulation/simulation.hpp>

#include "3d.hpp"
#include "result.h"

namespace {

std::vector<foeKeyYamlPair> exportComponents(foeEntityID entity, foeSimulation const *pSimulation) {
    std::vector<foeKeyYamlPair> keyDataPairs;

    auto *pPosition3dPool = (foePosition3dPool *)foeSimulationGetComponentPool(
        pSimulation, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL);
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

foeResult onRegister(foeExporter exporter) {
    foeResult result = to_foeResult(FOE_POSITION_YAML_SUCCESS);

    if (std::string_view{exporter.pName} == "Yaml") {
        // Components
        result = foeImexYamlRegisterComponentFn(exportComponents);
        if (result.value != FOE_SUCCESS) {
            result = to_foeResult(FOE_POSITION_YAML_ERROR_FAILED_TO_REGISTER_3D_EXPORTER);
            goto REGISTRATION_FAILED;
        }
    }

REGISTRATION_FAILED:
    if (result.value != FOE_SUCCESS)
        onDeregister(exporter);

    return result;
}

foeExportFunctionality exportFunctionality{
    .onRegister = onRegister,
    .onDeregister = onDeregister,
};

} // namespace

extern "C" foeResult foePositionYamlRegisterExporters() {
    return foeRegisterExportFunctionality(&exportFunctionality);
}

extern "C" void foePositionYamlDeregisterExporters() {
    foeDeregisterExportFunctionality(&exportFunctionality);
}