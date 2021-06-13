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

#include <foe/physics/yaml/import_registrar.hpp>

#include <foe/imex/yaml/generator.hpp>
#include <foe/physics/component/rigid_body_pool.hpp>
#include <foe/physics/resource/collision_shape.hpp>
#include <foe/physics/resource/collision_shape_loader.hpp>
#include <foe/physics/resource/collision_shape_pool.hpp>
#include <foe/yaml/exception.hpp>

#include "collision_shape.hpp"
#include "rigid_body.hpp"

namespace {

bool collisionShapeCreateProcessing(foeResourceID resource,
                                    foeResourceCreateInfoBase *pCreateInfo,
                                    std::vector<foeResourceLoaderBase *> &resourceLoaders,
                                    std::vector<foeResourcePoolBase *> &resourcePools) {
    foePhysCollisionShapePool *pCollisionShapePool{nullptr};
    for (auto &it : resourcePools) {
        pCollisionShapePool = dynamic_cast<foePhysCollisionShapePool *>(it);

        if (pCollisionShapePool != nullptr)
            break;
    }

    foePhysCollisionShapeLoader *pCollisionShapeLoader{nullptr};
    for (auto &it : resourceLoaders) {
        pCollisionShapeLoader = dynamic_cast<foePhysCollisionShapeLoader *>(it);

        if (pCollisionShapeLoader != nullptr)
            break;
    }

    if (pCollisionShapePool == nullptr || pCollisionShapeLoader == nullptr)
        return false;

    auto *pCollisionShape = new foePhysCollisionShape{resource, pCollisionShapeLoader};

    if (!pCollisionShapePool->add(pCollisionShape)) {
        delete pCollisionShape;
        return false;
    }

    return true;
}

bool importRigidBody(YAML::Node const &node,
                     foeIdGroupTranslator const *pGroupTranslator,
                     foeEntityID entity,
                     std::vector<foeComponentPoolBase *> &componentPools) {
    if (auto dataNode = node["rigid_body"]; dataNode) {
        foeRigidBodyPool *pPool;

        for (auto it : componentPools) {
            pPool = dynamic_cast<foeRigidBodyPool *>(it);
            if (pPool != nullptr)
                break;
        }

        if (pPool == nullptr)
            return false;

        try {
            foeRigidBody data = yaml_read_RigidBody(dataNode, pGroupTranslator);

            pPool->insert(entity, std::move(data));

            return true;
        } catch (foeYamlException const &e) {
            throw foeYamlException{"rigid_body::" + e.whatStr()};
        }
    }

    return false;
}

void onDeregister(foeImporterGenerator *pGenerator) {
    if (auto pYamlImporter = dynamic_cast<foeYamlImporterGenerator *>(pGenerator); pYamlImporter) {
        // Resources
        pYamlImporter->deregisterResourceFns("collision_shape_v1", yaml_read_collision_shape_definition,
                                      collisionShapeCreateProcessing);

        // Components
        pYamlImporter->deregisterComponentFn("rigid_body", importRigidBody);
    }
}

void onRegister(foeImporterGenerator *pGenerator) {
    if (auto pYamlImporter = dynamic_cast<foeYamlImporterGenerator *>(pGenerator); pYamlImporter) {
        // Resources
        if (!pYamlImporter->registerResourceFns("collision_shape_v1", yaml_read_collision_shape_definition,
                                        collisionShapeCreateProcessing))
            goto FAILED_TO_ADD;

        // Components
        pYamlImporter->registerComponentFn("rigid_body", importRigidBody);
    }

    return;

FAILED_TO_ADD:
    onDeregister(pGenerator);
}

} // namespace

bool foePhysicsRegisterYamlImportFunctionality() {
    return foeRegisterImportFunctionality(foeImportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}

void foePhysicsDeregisterYamlImportFunctionality() {
    foeDeregisterImportFunctionality(foeImportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}