/*
    Copyright (C) 2021 George Cave.

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

#include <foe/physics/yaml/export_registrar.hpp>

#include <foe/imex/exporters.hpp>
#include <foe/imex/yaml/exporter.hpp>
#include <foe/physics/component/rigid_body_pool.hpp>
#include <foe/physics/resource/collision_shape_loader.hpp>
#include <foe/physics/resource/collision_shape_pool.hpp>

#include "collision_shape.hpp"
#include "error_code.hpp"
#include "rigid_body.hpp"

namespace {

std::vector<foeKeyYamlPair> exportResources(foeResourceID resource,
                                            foeResourcePoolBase **pResourcePools,
                                            uint32_t resourcePoolCount) {
    std::vector<foeKeyYamlPair> keyDataPairs;
    auto const *pEndPools = pResourcePools + resourcePoolCount;

    for (; pResourcePools != pEndPools; ++pResourcePools) {
        auto *pCollisionShapePool = dynamic_cast<foeCollisionShapePool *>(*pResourcePools);
        if (pCollisionShapePool) {
            auto const *pCollisionShape = pCollisionShapePool->find(resource);
            if (pCollisionShape && pCollisionShape->pCreateInfo) {
                if (auto dynPtr = dynamic_cast<foeCollisionShapeCreateInfo *>(
                        pCollisionShape->pCreateInfo.get());
                    dynPtr)
                    keyDataPairs.emplace_back(foeKeyYamlPair{
                        .key = yaml_collision_shape_key(),
                        .data = yaml_write_collision_shape(*dynPtr),
                    });
            }
        }
    }

    return keyDataPairs;
}

std::vector<foeKeyYamlPair> exportComponents(foeEntityID entity,
                                             foeComponentPoolBase **pComponentPools,
                                             uint32_t componentPoolCount) {
    std::vector<foeKeyYamlPair> keyDataPairs;
    auto const *pEndPools = pComponentPools + componentPoolCount;

    for (; pComponentPools != pEndPools; ++pComponentPools) {
        auto *pRigidBodyPool = dynamic_cast<foeRigidBodyPool *>(*pComponentPools);
        if (pRigidBodyPool) {
            if (auto offset = pRigidBodyPool->find(entity); offset != pRigidBodyPool->size()) {
                keyDataPairs.emplace_back(foeKeyYamlPair{
                    .key = yaml_rigid_body_key(),
                    .data = yaml_write_rigid_body(*(pRigidBodyPool->begin<1>() + offset)),
                });
            }
        }
    }

    return keyDataPairs;
}

void onDeregister(foeExporterBase *pExporter) {
    auto *pYamlExporter = dynamic_cast<foeYamlExporter *>(pExporter);
    if (pYamlExporter) {
        // Resources
        pYamlExporter->deregisterResourceFn(exportResources);

        // Components
        pYamlExporter->deregisterComponentFn(exportComponents);
    }
}

std::error_code onRegister(foeExporterBase *pExporter) {
    std::error_code errC;

    auto *pYamlExporter = dynamic_cast<foeYamlExporter *>(pExporter);
    if (pYamlExporter) {
        // Resources
        if (!pYamlExporter->registerResourceFn(exportResources)) {
            errC = FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_COLLISION_SHAPE_EXPORTER;
            goto REGISTRATION_FAILED;
        }

        // Components
        if (!pYamlExporter->registerComponentFn(exportComponents)) {
            errC = FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_RIGID_BODY_EXPORTER;
            goto REGISTRATION_FAILED;
        }
    }

REGISTRATION_FAILED:
    if (errC)
        onDeregister(pExporter);

    return errC;
}

} // namespace

auto foePhysicsRegisterYamlExportFunctionality() -> std::error_code {
    return foeRegisterExportFunctionality(foeExportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}

void foePhysicsDeregisterYamlExportFunctionality() {
    foeDeregisterExportFunctionality(foeExportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}