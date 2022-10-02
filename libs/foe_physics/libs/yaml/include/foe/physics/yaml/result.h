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
    FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_COLLISION_SHAPE_IMPORTER = -1000019001,
    FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_COLLISION_SHAPE_EXPORTER = -1000019002,
    FOE_PHYSICS_YAML_ERROR_COLLISION_SHAPE_POOL_NOT_FOUND = -1000019003,
    FOE_PHYSICS_YAML_ERROR_COLLISION_SHAPE_ALREADY_EXISTS = -1000019004,
    FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_RIGID_BODY_IMPORTER = -1000019005,
    FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_RIGID_BODY_EXPORTER = -1000019006,
} foePhysicsYamlResult;

FOE_PHYSICS_YAML_EXPORT void foePhysicsYamlResultToString(foePhysicsYamlResult value,
                                                          char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_PHYSICS_YAML_RESULT_H