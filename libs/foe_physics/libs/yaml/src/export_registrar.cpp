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
#include <foe/physics/resource/collision_shape.hpp>
#include <foe/physics/resource/collision_shape_pool.hpp>

#include "collision_shape.hpp"
#include <foe/physics/yaml/component/rigid_body.hpp>

namespace {

std::vector<foeKeyYamlPair> exportResources(foeResourceID resource,
                                            foeResourcePoolBase **pResourcePools,
                                            uint32_t resourcePoolCount) {
    std::vector<foeKeyYamlPair> keyDataPairs;
    auto const *pEndPools = pResourcePools + resourcePoolCount;

    for (; pResourcePools != pEndPools; ++pResourcePools) {
        auto *pCollisionShapePool = dynamic_cast<foePhysCollisionShapePool *>(*pResourcePools);
        if (pCollisionShapePool) {
            auto const *pCollisionShape = pCollisionShapePool->find(resource);
            if (pCollisionShape && pCollisionShape->createInfo) {
                keyDataPairs.emplace_back(foeKeyYamlPair{
                    .key = "collision_shape_v1",
                    .data =
                        yaml_write_collision_shape_definition(*pCollisionShape->createInfo.get()),
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
                    .key = "rigid_body",
                    .data = yaml_write_RigidBody(*(pRigidBodyPool->begin<1>() + offset)),
                });
            }
        }
    }

    return keyDataPairs;
}

void onRegister(foeExporterBase *pExporter) {
    auto *pYamlExporter = dynamic_cast<foeYamlExporter *>(pExporter);
    if (pYamlExporter) {
        // Resource
        pYamlExporter->registerResourceFn(exportResources);

        // Component
        pYamlExporter->registerComponentFn(exportComponents);
    }
}

void onDeregister(foeExporterBase *pExporter) {
    auto *pYamlExporter = dynamic_cast<foeYamlExporter *>(pExporter);
    if (pYamlExporter) {
        // Resource
        pYamlExporter->deregisterResourceFn(exportResources);

        // Component
        pYamlExporter->deregisterComponentFn(exportComponents);
    }
}

} // namespace

void foePhysicsRegisterYamlExportFunctionality() {
    foeRegisterExportFunctionality(foeExportFunctionality{
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