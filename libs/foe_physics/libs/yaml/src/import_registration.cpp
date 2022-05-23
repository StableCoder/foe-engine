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
#include <foe/physics/type_defs.h>
#include <foe/resource/pool.h>
#include <foe/simulation/simulation.hpp>
#include <foe/yaml/exception.hpp>

#include "collision_shape.hpp"
#include "result.h"
#include "rigid_body.hpp"

namespace {

foeResult collisionShapeCreateProcessing(foeResourceID resourceID,
                                         foeResourceCreateInfo createInfo,
                                         foeSimulation const *pSimulation) {
    foeResource collisionShape =
        foeResourcePoolAdd(pSimulation->resourcePool, resourceID,
                           FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE, sizeof(foeCollisionShape));

    if (collisionShape == FOE_NULL_HANDLE)
        return to_foeResult(FOE_PHYSICS_YAML_ERROR_COLLISION_SHAPE_ALREADY_EXISTS);

    return to_foeResult(FOE_PHYSICS_YAML_SUCCESS);
}

bool importRigidBody(YAML::Node const &node,
                     foeEcsGroupTranslator groupTranslator,
                     foeEntityID entity,
                     foeSimulation const *pSimulation) {
    if (auto dataNode = node[yaml_rigid_body_key()]; dataNode) {
        auto *pPool = (foeRigidBodyPool *)foeSimulationGetComponentPool(
            pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL);

        if (pPool == nullptr)
            return false;

        try {
            foeRigidBody data = yaml_read_rigid_body(dataNode, groupTranslator);

            pPool->insert(entity, std::move(data));

            return true;
        } catch (foeYamlException const &e) {
            throw foeYamlException{std::string{yaml_rigid_body_key()} + "::" + e.whatStr()};
        }
    }

    return false;
}

} // namespace

extern "C" foeResult foePhysicsYamlRegisterImporters() {
    foeResult result = to_foeResult(FOE_PHYSICS_YAML_SUCCESS);

    // Resources
    if (!foeImexYamlRegisterResourceFns(yaml_collision_shape_key(), yaml_read_collision_shape,
                                        collisionShapeCreateProcessing)) {
        result = to_foeResult(FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_COLLISION_SHAPE_IMPORTER);
        goto REGISTRATION_FAILED;
    }

    // Components
    if (!foeImexYamlRegisterComponentFn(yaml_rigid_body_key(), importRigidBody)) {
        result = to_foeResult(FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_RIGID_BODY_IMPORTER);
        goto REGISTRATION_FAILED;
    }

REGISTRATION_FAILED:
    if (result.value != FOE_SUCCESS)
        foePhysicsYamlDeregisterImporters();

    return result;
}

extern "C" void foePhysicsYamlDeregisterImporters() {
    // Components
    foeImexYamlDeregisterComponentFn(yaml_rigid_body_key(), importRigidBody);

    // Resources
    foeImexYamlDeregisterResourceFns(yaml_collision_shape_key(), yaml_read_collision_shape,
                                     collisionShapeCreateProcessing);
}