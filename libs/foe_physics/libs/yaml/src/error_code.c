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

#include <foe/physics/yaml/error_code.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        pResultStr = #X;                                                                           \
        break;

void foePhysicsYamlResultToString(foePhysicsYamlResult value,
                                  char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = NULL;

    switch (value) {
        RESULT_CASE(FOE_PHYSICS_YAML_SUCCESS)
        // CollisionShape Resource
        RESULT_CASE(FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_COLLISION_SHAPE_IMPORTER)
        RESULT_CASE(FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_COLLISION_SHAPE_EXPORTER)
        RESULT_CASE(FOE_PHYSICS_YAML_ERROR_COLLISION_SHAPE_POOL_NOT_FOUND)
        RESULT_CASE(FOE_PHYSICS_YAML_ERROR_COLLISION_SHAPE_ALREADY_EXISTS)
        // RigidBody Component
        RESULT_CASE(FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_RIGID_BODY_IMPORTER)
        RESULT_CASE(FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_RIGID_BODY_EXPORTER)

    default:
        if (value > 0) {
            sprintf(buffer, "FOE_PHYSICS_YAML_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            sprintf(buffer, "FOE_PHYSICS_YAML_UNKNOWN_ERROR_%i", value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}
