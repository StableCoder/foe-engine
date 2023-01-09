// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/position/yaml/export_registration.h>

#include <foe/imex/exporters.h>
#include <foe/imex/yaml/exporter.hpp>
#include <foe/position/component/3d_pool.h>
#include <foe/position/type_defs.h>
#include <foe/simulation/simulation.hpp>

#include "3d.hpp"
#include "result.h"

namespace {

std::vector<foeKeyYamlPair> exportComponents(foeEntityID entity, foeSimulation const *pSimulation) {
    std::vector<foeKeyYamlPair> keyDataPairs;

    foePosition3dPool componentPool = (foePosition3dPool)foeSimulationGetComponentPool(
        pSimulation, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL);
    if (componentPool != FOE_NULL_HANDLE) {
        foeEntityID const *const pStartID = foeEcsComponentPoolIdPtr(componentPool);
        foeEntityID const *const pEndID = pStartID + foeEcsComponentPoolSize(componentPool);

        foeEntityID const *pID = std::lower_bound(pStartID, pEndID, entity);

        if (pID != pEndID && *pID == entity) {
            size_t offset = pID - pStartID;

            foePosition3d **ppPositionData =
                (foePosition3d **)foeEcsComponentPoolDataPtr(componentPool) + offset;

            keyDataPairs.emplace_back(foeKeyYamlPair{
                .key = yaml_position3d_key(),
                .data = yaml_write_position3d(**ppPositionData),
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

foeResultSet onRegister(foeExporter exporter) {
    foeResultSet result = to_foeResult(FOE_POSITION_YAML_SUCCESS);

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

extern "C" foeResultSet foePositionYamlRegisterExporters() {
    return foeRegisterExportFunctionality(&exportFunctionality);
}

extern "C" void foePositionYamlDeregisterExporters() {
    foeDeregisterExportFunctionality(&exportFunctionality);
}