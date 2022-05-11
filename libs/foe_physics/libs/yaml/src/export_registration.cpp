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

#include <foe/physics/yaml/export_registration.h>

#include <foe/imex/exporters.hpp>
#include <foe/imex/yaml/exporter.hpp>
#include <foe/physics/component/rigid_body_pool.hpp>
#include <foe/physics/resource/collision_shape_loader.hpp>
#include <foe/physics/type_defs.h>
#include <foe/resource/pool.h>
#include <foe/simulation/simulation.hpp>

#include "collision_shape.hpp"
#include "error_code.hpp"
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

std::error_code onRegister(foeExporter exporter) {
    std::error_code errC;

    if (std::string_view{exporter.pName} == "Yaml") {
        // Resources
        if (foeImexYamlRegisterResourceFn(exportResources)) {
            errC = FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_COLLISION_SHAPE_EXPORTER;
            goto REGISTRATION_FAILED;
        }

        // Components
        if (foeImexYamlRegisterComponentFn(exportComponents)) {
            errC = FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_RIGID_BODY_EXPORTER;
            goto REGISTRATION_FAILED;
        }
    }
REGISTRATION_FAILED:
    if (errC)
        onDeregister(exporter);

    return errC;
}

} // namespace

extern "C" foeErrorCode foePhysicsYamlRegisterExporters() {
    auto errC = foeRegisterExportFunctionality(foeExportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });

    return foeToErrorCode(errC);
}

extern "C" void foePhysicsYamlDeregisterExporters() {
    foeDeregisterExportFunctionality(foeExportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}