/*
    Copyright (C) 2022 George Cave.

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

#ifndef FOE_PHYSICS_YAML_ERROR_CODE_H
#define FOE_PHYSICS_YAML_ERROR_CODE_H

#include <foe/error_code.h>
#include <foe/physics/yaml/export.h>

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

#endif // FOE_PHYSICS_YAML_ERROR_CODE_H