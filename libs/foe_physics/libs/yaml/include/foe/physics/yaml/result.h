// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_PHYSICS_YAML_RESULT_H
#define FOE_PHYSICS_YAML_RESULT_H

#include <foe/physics/yaml/export.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foePhysicsYamlResult {
    FOE_PHYSICS_YAML_SUCCESS = 0,
    // CollisionShape Resource
    FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_COLLISION_SHAPE_IMPORTER,
    FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_COLLISION_SHAPE_EXPORTER,
    FOE_PHYSICS_YAML_ERROR_COLLISION_SHAPE_POOL_NOT_FOUND,
    FOE_PHYSICS_YAML_ERROR_COLLISION_SHAPE_ALREADY_EXISTS,
    // RigidBody Component
    FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_RIGID_BODY_IMPORTER,
    FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_RIGID_BODY_EXPORTER,

    // Need to have a negative enum value to prevent treatment as a flag
    FOE_PHYSICS_ERROR_NEGATIVE_VALUE = FOE_RESULT_MIN_ENUM,
} foePhysicsYamlResult;

FOE_PHYSICS_YAML_EXPORT void foePhysicsYamlResultToString(foePhysicsYamlResult value,
                                                          char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_PHYSICS_YAML_RESULT_H