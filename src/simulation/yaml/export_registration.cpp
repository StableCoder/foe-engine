// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "export_registration.h"

#include <foe/imex/exporters.h>
#include <foe/imex/yaml/exporter.hpp>
#include <foe/resource/pool.h>
#include <foe/simulation/simulation.hpp>

#include "../armature_state_imex.hpp"
#include "../armature_state_pool.hpp"
#include "../render_state_imex.hpp"
#include "../render_state_pool.h"
#include "../type_defs.h"
#include "armature.hpp"
#include "result.h"

namespace {

std::vector<foeKeyYamlPair> exportResources(foeResourceCreateInfo createInfo) {
    std::vector<foeKeyYamlPair> keyDataPairs;

    if (createInfo == FOE_NULL_HANDLE)
        return keyDataPairs;

    // foeArmature
    if (foeResourceCreateInfoGetType(createInfo) ==
        FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_CREATE_INFO) {
        auto const *pCreateInfo =
            (foeArmatureCreateInfo const *)foeResourceCreateInfoGetData(createInfo);

        keyDataPairs.emplace_back(foeKeyYamlPair{
            .key = yaml_armature_key(),
            .data = yaml_write_armature(*pCreateInfo),
        });
    }

    return keyDataPairs;
}

std::vector<foeKeyYamlPair> exportComponents(foeEntityID entity, foeSimulation const *pSimulation) {
    std::vector<foeKeyYamlPair> keyDataPairs;

    // ArmatureState
    foeArmatureStatePool armatureStatePool = (foeArmatureStatePool)foeSimulationGetComponentPool(
        pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL);
    if (armatureStatePool) {
        foeEntityID const *const pStartID = foeEcsComponentPoolIdPtr(armatureStatePool);
        foeEntityID const *const pEndID = pStartID + foeEcsComponentPoolSize(armatureStatePool);

        foeEntityID const *pID = std::lower_bound(pStartID, pEndID, entity);

        if (pID != pEndID && *pID == entity) {
            size_t offset = pID - pStartID;
            foeArmatureState *pArmatureState =
                (foeArmatureState *)foeEcsComponentPoolDataPtr(armatureStatePool) + offset;

            keyDataPairs.emplace_back(foeKeyYamlPair{
                .key = yaml_armature_state_key(),
                .data = yaml_write_ArmatureState(*pArmatureState),
            });
        }
    }

    // RenderState
    foeRenderStatePool renderStatePool = (foeRenderStatePool)foeSimulationGetComponentPool(
        pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL);
    if (renderStatePool) {
        foeEntityID const *const pStartID = foeEcsComponentPoolIdPtr(renderStatePool);
        foeEntityID const *const pEndID = pStartID + foeEcsComponentPoolSize(renderStatePool);

        foeEntityID const *pID = std::lower_bound(pStartID, pEndID, entity);

        if (pID != pEndID && *pID == entity) {
            size_t offset = pID - pStartID;
            foeRenderState *pRenderState =
                (foeRenderState *)foeEcsComponentPoolDataPtr(renderStatePool) + offset;

            keyDataPairs.emplace_back(foeKeyYamlPair{
                .key = yaml_render_state_key(),
                .data = yaml_write_RenderState(*pRenderState),
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

foeResultSet onRegister(foeExporter exporter) {
    foeResultSet result = to_foeResult(FOE_BRINGUP_YAML_SUCCESS);

    if (std::string_view{exporter.pName} == "Yaml") {
        // Resource
        result = foeImexYamlRegisterResourceFn(exportResources);
        if (result.value != FOE_SUCCESS) {
            result = to_foeResult(FOE_BRINGUP_YAML_ERROR_FAILED_TO_REGISTER_RESOURCE_EXPORTERS);
            goto REGISTRATION_FAILED;
        }

        // Component
        result = foeImexYamlRegisterComponentFn(exportComponents);
        if (result.value != FOE_SUCCESS) {
            result = to_foeResult(FOE_BRINGUP_YAML_ERROR_FAILED_TO_REGISTER_COMPONENT_EXPORTERS);
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

extern "C" foeResultSet foeBringupYamlRegisterExporters() {
    return foeRegisterExportFunctionality(&exportFunctionality);
}

extern "C" void foeBringupYamlDeregisterExporters() {
    foeDeregisterExportFunctionality(&exportFunctionality);
}