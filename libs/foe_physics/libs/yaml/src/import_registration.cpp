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

#include <foe/physics/yaml/import_registration.h>

#include <foe/imex/yaml/importer.hpp>
#include <foe/physics/component/rigid_body_pool.hpp>
#include <foe/physics/resource/collision_shape.hpp>
#include <foe/physics/resource/collision_shape_loader.hpp>
#include <foe/physics/resource/collision_shape_pool.hpp>
#include <foe/physics/type_defs.h>
#include <foe/yaml/exception.hpp>

#include "collision_shape.hpp"
#include "error_code.hpp"
#include "rigid_body.hpp"

namespace {

std::error_code collisionShapeCreateProcessing(foeResourceID resource,
                                               foeResourceCreateInfo createInfo,
                                               foeSimulation const *pSimulation) {
    auto *pCollisionShapePool = (foeCollisionShapePool *)foeSimulationGetResourcePool(
        pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_POOL);

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
                     foeSimulation const *pSimulation) {
    if (auto dataNode = node[yaml_rigid_body_key()]; dataNode) {
        auto *pPool = (foeRigidBodyPool *)foeSimulationGetComponentPool(
            pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL);

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

} // namespace

extern "C" foeErrorCode foePhysicsYamlRegisterImporters() {
    std::error_code errC;

    // Resources
    if (!foeImexYamlRegisterResourceFns(yaml_collision_shape_key(), yaml_read_collision_shape,
                                        collisionShapeCreateProcessing)) {
        errC = FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_COLLISION_SHAPE_IMPORTER;
        goto REGISTRATION_FAILED;
    }

    // Components
    if (!foeImexYamlRegisterComponentFn(yaml_rigid_body_key(), importRigidBody)) {
        errC = FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_RIGID_BODY_IMPORTER;
        goto REGISTRATION_FAILED;
    }

REGISTRATION_FAILED:
    if (errC)
        foePhysicsYamlDeregisterImporters();

    return foeToErrorCode(errC);
}

extern "C" void foePhysicsYamlDeregisterImporters() {
    // Components
    foeImexYamlDeregisterComponentFn(yaml_rigid_body_key(), importRigidBody);

    // Resources
    foeImexYamlDeregisterResourceFns(yaml_collision_shape_key(), yaml_read_collision_shape,
                                     collisionShapeCreateProcessing);
}