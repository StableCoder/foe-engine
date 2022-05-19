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

typedef enum foeBringupResult {
    FOE_BRINGUP_SUCCESS = 0,

    FOE_BRINGUP_INITIALIZATION_FAILED,
    FOE_BRINGUP_NOT_INITIALIZED,
    FOE_BRINGUP_FAILED_TO_LOAD_PLUGIN,

    // Loaders
    FOE_BRINGUP_ERROR_LOADER_INITIALIZATION_FAILED,
    FOE_BRINGUP_ERROR_INCOMPATIBLE_CREATE_INFO,
    FOE_BRINGUP_ERROR_IMPORT_FAILED,

    FOE_BRINGUP_ERROR_FAILED_TO_REGISTER_EXPORTERS,

    FOE_BRINGUP_ERROR_NO_PHYSICAL_DEVICE_MEETS_REQUIREMENTS,
    // RenderGraph - RenderScene
    FOE_BRINGUP_RENDER_SCENE_COLOUR_TARGET_NOT_IMAGE,
    FOE_BRINGUP_RENDER_SCENE_COLOUR_TARGET_NOT_MUTABLE,
    FOE_BRINGUP_RENDER_SCENE_COLOUR_TARGET_NO_STATE,
    FOE_BRINGUP_RENDER_SCENE_DEPTH_TARGET_NOT_IMAGE,
    FOE_BRINGUP_RENDER_SCENE_DEPTH_TARGET_NOT_MUTABLE,
    FOE_BRINGUP_RENDER_SCENE_DEPTH_TARGET_NO_STATE,
    // Armature System
    FOE_BRINGUP_ERROR_NO_ARMATURE_POOL_PROVIDED,
    FOE_BRINGUP_ERROR_NO_ARMATURE_STATE_POOL_PROVIDED,

    // Need to have a negative enum value to prevent treatment as a flag
    FOE_BRINGUP_ERROR_NEGATIVE_VALUE = FOE_RESULT_MIN_ENUM,
} foeBringupResult;

void foeBringupResultToString(foeBringupResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // ERROR_CODE_H