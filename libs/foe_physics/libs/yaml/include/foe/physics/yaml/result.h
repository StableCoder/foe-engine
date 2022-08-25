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
    FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_COLLISION_SHAPE_IMPORTER = -1,
    FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_COLLISION_SHAPE_EXPORTER = -2,
    FOE_PHYSICS_YAML_ERROR_COLLISION_SHAPE_POOL_NOT_FOUND = -3,
    FOE_PHYSICS_YAML_ERROR_COLLISION_SHAPE_ALREADY_EXISTS = -4,
    FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_RIGID_BODY_IMPORTER = -5,
    FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_RIGID_BODY_EXPORTER = -6,
} foePhysicsYamlResult;

FOE_PHYSICS_YAML_EXPORT void foePhysicsYamlResultToString(foePhysicsYamlResult value,
                                                          char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_PHYSICS_YAML_RESULT_H