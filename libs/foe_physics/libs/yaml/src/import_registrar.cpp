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
#include "error_code.hpp"
#include "rigid_body.hpp"

namespace {

std::error_code collisionShapeCreateProcessing(
    foeResourceID resource,
    foeResourceCreateInfoBase *pCreateInfo,
    std::vector<foeResourceLoaderBase *> &resourceLoaders,
    std::vector<foeResourcePoolBase *> &resourcePools) {
    foeCollisionShapePool *pCollisionShapePool{nullptr};
    for (auto &it : resourcePools) {
        pCollisionShapePool = dynamic_cast<foeCollisionShapePool *>(it);

        if (pCollisionShapePool != nullptr)
            break;
    }

    if (pCollisionShapePool == nullptr)
        return FOE_PHYSICS_YAML_ERROR_COLLISION_SHAPE_POOL_NOT_FOUND;

    auto *pCollisionShape = pCollisionShapePool->add(resource);

    if (!pCollisionShape)
        return FOE_PHYSICS_YAML_ERROR_COLLISION_SHAPE_ALREADY_EXISTS;

    return FOE_PHYSICS_YAML_SUCCESS;
}

bool importRigidBody(YAML::Node const &node,
                     foeIdGroupTranslator const *pGroupTranslator,
                     foeEntityID entity,
                     std::vector<foeComponentPoolBase *> &componentPools) {
    if (auto dataNode = node[yaml_rigid_body_key()]; dataNode) {
        foeRigidBodyPool *pPool;

        for (auto it : componentPools) {
            pPool = dynamic_cast<foeRigidBodyPool *>(it);
            if (pPool != nullptr)
                break;
        }

        if (pPool == nullptr)
            return false;

        try {
            foeRigidBody data = yaml_read_rigid_body(dataNode, pGroupTranslator);

            pPool->insert(entity, std::move(data));

            return true;
        } catch (foeYamlException const &e) {
            throw foeYamlException{std::string{yaml_rigid_body_key()} + "::" + e.whatStr()};
        }
    }

    return false;
}

void onDeregister(foeImporterGenerator *pGenerator) {
    if (auto pYamlImporter = dynamic_cast<foeYamlImporterGenerator *>(pGenerator); pYamlImporter) {
        // Resources
        pYamlImporter->deregisterResourceFns(yaml_collision_shape_key(), yaml_read_collision_shape,
                                             collisionShapeCreateProcessing);

        // Components
        pYamlImporter->deregisterComponentFn(yaml_rigid_body_key(), importRigidBody);
    }
}

std::error_code onRegister(foeImporterGenerator *pGenerator) {
    std::error_code errC;

    if (auto pYamlImporter = dynamic_cast<foeYamlImporterGenerator *>(pGenerator); pYamlImporter) {
        // Resources
        if (!pYamlImporter->registerResourceFns(yaml_collision_shape_key(),
                                                yaml_read_collision_shape,
                                                collisionShapeCreateProcessing)) {
            errC = FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_COLLISION_SHAPE_IMPORTER;
            goto REGISTRATION_FAILED;
        }

        // Components
        if (!pYamlImporter->registerComponentFn(yaml_rigid_body_key(), importRigidBody)) {
            errC = FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_RIGID_BODY_IMPORTER;
            goto REGISTRATION_FAILED;
        }
    }

REGISTRATION_FAILED:
    if (errC)
        onDeregister(pGenerator);

    return errC;
}

} // namespace

auto foePhysicsRegisterYamlImportFunctionality() -> std::error_code {
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