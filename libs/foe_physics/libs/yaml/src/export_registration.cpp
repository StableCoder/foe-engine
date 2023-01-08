// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/physics/yaml/export_registration.h>

#include <foe/imex/exporters.h>
#include <foe/imex/yaml/exporter.hpp>
#include <foe/physics/component/rigid_body_pool.h>
#include <foe/physics/type_defs.h>
#include <foe/resource/pool.h>
#include <foe/simulation/simulation.hpp>

#include "collision_shape.hpp"
#include "result.h"
#include "rigid_body.hpp"

namespace {

std::vector<foeKeyYamlPair> exportResources(foeResourceCreateInfo createInfo) {
    std::vector<foeKeyYamlPair> keyDataPairs;

    if (createInfo == FOE_NULL_HANDLE)
        return keyDataPairs;

    if (foeResourceCreateInfoGetType(createInfo) ==
        FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_CREATE_INFO) {
        auto const *pCreateInfo =
            (foeCollisionShapeCreateInfo const *)foeResourceCreateInfoGetData(createInfo);

        keyDataPairs.emplace_back(foeKeyYamlPair{
            .key = yaml_collision_shape_key(),
            .data = yaml_write_collision_shape(*pCreateInfo),
        });
    }

    return keyDataPairs;
}

std::vector<foeKeyYamlPair> exportComponents(foeEntityID entity, foeSimulation const *pSimulation) {
    std::vector<foeKeyYamlPair> keyDataPairs;

    foeRigidBodyPool rigidBodyPool = (foeRigidBodyPool)foeSimulationGetComponentPool(
        pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL);
    if (rigidBodyPool != FOE_NULL_HANDLE) {
        foeEntityID const *const pStartID = foeEcsComponentPoolIdPtr(rigidBodyPool);
        foeEntityID const *const pEndID = pStartID + foeEcsComponentPoolSize(rigidBodyPool);

        foeEntityID const *pID = std::lower_bound(pStartID, pEndID, entity);

        if (pID != pEndID && *pID == entity) {
            size_t offset = pID - pStartID;

            foeRigidBody *pRigidBodyData =
                (foeRigidBody *)foeEcsComponentPoolDataPtr(rigidBodyPool) + offset;

            keyDataPairs.emplace_back(foeKeyYamlPair{
                .key = yaml_rigid_body_key(),
                .data = yaml_write_rigid_body(*pRigidBodyData),
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

foeResultSet onRegister(foeExporter exporter) {
    foeResultSet result = to_foeResult(FOE_PHYSICS_YAML_SUCCESS);

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

extern "C" foeResultSet foePhysicsYamlRegisterExporters() {
    return foeRegisterExportFunctionality(&exportFunctionality);
}

extern "C" void foePhysicsYamlDeregisterExporters() {
    foeDeregisterExportFunctionality(&exportFunctionality);
}