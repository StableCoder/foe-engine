// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/physics/yaml/import_registration.h>

#include <foe/imex/yaml/importer.hpp>
#include <foe/physics/component/rigid_body_pool.h>
#include <foe/physics/resource/collision_shape.hpp>
#include <foe/physics/type_defs.h>
#include <foe/resource/pool.h>
#include <foe/simulation/simulation.hpp>
#include <foe/yaml/exception.hpp>

#include "collision_shape.hpp"
#include "result.h"
#include "rigid_body.hpp"

namespace {

foeResultSet collisionShapeCreateProcessing(foeResourceID resourceID,
                                            foeResourceCreateInfo createInfo,
                                            foeSimulation const *pSimulation) {
    foeResource collisionShape =
        foeResourcePoolAdd(pSimulation->resourcePool, resourceID,
                           FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE, sizeof(foeCollisionShape));

    if (collisionShape == FOE_NULL_HANDLE)
        return to_foeResult(FOE_PHYSICS_YAML_ERROR_COLLISION_SHAPE_ALREADY_EXISTS);

    foeResourceDecrementRefCount(collisionShape);
    return to_foeResult(FOE_PHYSICS_YAML_SUCCESS);
}

bool importRigidBody(YAML::Node const &node,
                     foeEcsGroupTranslator groupTranslator,
                     foeEntityID entity,
                     foeSimulation const *pSimulation) {
    if (auto dataNode = node[yaml_rigid_body_key()]; dataNode) {
        foeRigidBodyPool pool = (foeRigidBodyPool)foeSimulationGetComponentPool(
            pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL);

        if (pool == FOE_NULL_HANDLE)
            return false;

        try {
            foeRigidBody data = yaml_read_rigid_body(dataNode, groupTranslator);

            foeEcsComponentPoolInsert(pool, entity, &data);

            return true;
        } catch (foeYamlException const &e) {
            throw foeYamlException{std::string{yaml_rigid_body_key()} + "::" + e.whatStr()};
        }
    }

    return false;
}

} // namespace

extern "C" foeResultSet foePhysicsYamlRegisterImporters() {
    foeResultSet result = to_foeResult(FOE_PHYSICS_YAML_SUCCESS);

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