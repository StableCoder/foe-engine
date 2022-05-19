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

#ifndef ERROR_CODE_H
#define ERROR_CODE_H

#include <foe/error_code.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foeBringupYamlResult {
    FOE_BRINGUP_YAML_SUCCESS = 0,
    FOE_BRINGUP_YAML_ERROR_UNSPECIFIED,

    // Importers
    FOE_BRINGUP_YAML_ERROR_ARMATURE_POOL_NOT_FOUND,
    FOE_BRINGUP_YAML_ERROR_ARMATURE_RESOURCE_ALREADY_EXISTS,

    FOE_BRINGUP_YAML_ERROR_FAILED_TO_REGISTER_ARMATURE_IMPORTER,
    FOE_BRINGUP_YAML_ERROR_FAILED_TO_REGISTER_ARMATURE_STATE_IMPORTER,
    FOE_BRINGUP_YAML_ERROR_FAILED_TO_REGISTER_RENDER_STATE_IMPORTER,
    FOE_BRINGUP_YAML_ERROR_FAILED_TO_REGISTER_CAMERA_IMPORTER,

    // Exporters
    FOE_BRINGUP_YAML_ERROR_FAILED_TO_REGISTER_RESOURCE_EXPORTERS,
    FOE_BRINGUP_YAML_ERROR_FAILED_TO_REGISTER_COMPONENT_EXPORTERS,

    // Need to have a negative enum value to prevent treatment as a flag
    FOE_BRINGUP_YAML_ERROR_NEGATIVE_VALUE = FOE_RESULT_MIN_ENUM,
} foeBringupYamlResult;

void foeBringupYamlResultToString(foeBringupYamlResult value,
                                  char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // ERROR_CODE_H