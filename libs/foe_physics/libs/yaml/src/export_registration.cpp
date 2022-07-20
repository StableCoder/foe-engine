// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/physics/yaml/export_registration.h>

#include <foe/imex/exporters.h>
#include <foe/imex/yaml/exporter.hpp>
#include <foe/physics/component/rigid_body_pool.hpp>
#include <foe/physics/resource/collision_shape_loader.hpp>
#include <foe/physics/type_defs.h>
#include <foe/resource/pool.h>
#include <foe/simulation/simulation.hpp>

#include "collision_shape.hpp"
#include "result.h"
#include "rigid_body.hpp"

namespace {

std::vector<foeKeyYamlPair> exportResources(foeResourceID resourceID,
                                            foeSimulation const *pSimulation) {
    std::vector<foeKeyYamlPair> keyDataPairs;

    foeResource resource = foeResourcePoolFind(pSimulation->resourcePool, resourceID);

    if (resource == FOE_NULL_HANDLE)
        return keyDataPairs;

    if (foeResourceGetType(resource) == FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE) {
        auto createInfo = foeResourceGetCreateInfo(resource);

        if (foeResourceCreateInfoGetType(createInfo) ==
            FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_CREATE_INFO) {
            auto const *pCreateInfo =
                (foeCollisionShapeCreateInfo const *)foeResourceCreateInfoGetData(createInfo);

            keyDataPairs.emplace_back(foeKeyYamlPair{
                .key = yaml_collision_shape_key(),
                .data = yaml_write_collision_shape(*pCreateInfo),
            });
        }
    }

    return keyDataPairs;
}

std::vector<foeKeyYamlPair> exportComponents(foeEntityID entity, foeSimulation const *pSimulation) {
    std::vector<foeKeyYamlPair> keyDataPairs;

    auto *pRigidBodyPool = (foeRigidBodyPool *)foeSimulationGetComponentPool(
        pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL);
    if (pRigidBodyPool) {
        if (auto offset = pRigidBodyPool->find(entity); offset != pRigidBodyPool->size()) {
            keyDataPairs.emplace_back(foeKeyYamlPair{
                .key = yaml_rigid_body_key(),
                .data = yaml_write_rigid_body(*(pRigidBodyPool->begin<1>() + offset)),
            });
        }
    }

    return keyDataPairs;
}

void onDeregister(foeExporter exporter) {
    if (std::string_view{exporter.pName} == "Yaml") {
        // Resources
        foeImexYamlDeregisterResourceFn(exportResources);

        // Components
        foeImexYamlDeregisterComponentFn(exportComponents);
    }
}

foeResult onRegister(foeExporter exporter) {
    foeResult result = to_foeResult(FOE_PHYSICS_YAML_SUCCESS);

    if (std::string_view{exporter.pName} == "Yaml") {
        // Resources
        result = foeImexYamlRegisterResourceFn(exportResources);
        if (result.value != FOE_SUCCESS) {
            result =
                to_foeResult(FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_COLLISION_SHAPE_EXPORTER);
            goto REGISTRATION_FAILED;
        }

        // Components
        result = foeImexYamlRegisterComponentFn(exportComponents);
        if (result.value != FOE_SUCCESS) {
            result = to_foeResult(FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_RIGID_BODY_EXPORTER);
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

extern "C" foeResult foePhysicsYamlRegisterExporters() {
    return foeRegisterExportFunctionality(&exportFunctionality);
}

extern "C" void foePhysicsYamlDeregisterExporters() {
    foeDeregisterExportFunctionality(&exportFunctionality);
}