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

#include "error_code.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        pResultStr = #X;                                                                           \
        break;

void foeBringupResultToString(foeBringupResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = NULL;

    switch (value) {
        RESULT_CASE(FOE_BRINGUP_SUCCESS)

        RESULT_CASE(FOE_BRINGUP_INITIALIZATION_FAILED)
        RESULT_CASE(FOE_BRINGUP_NOT_INITIALIZED)
        RESULT_CASE(FOE_BRINGUP_FAILED_TO_LOAD_PLUGIN)

        // Loaders
        RESULT_CASE(FOE_BRINGUP_ERROR_LOADER_INITIALIZATION_FAILED)
        RESULT_CASE(FOE_BRINGUP_ERROR_INCOMPATIBLE_CREATE_INFO)
        RESULT_CASE(FOE_BRINGUP_ERROR_IMPORT_FAILED)

        RESULT_CASE(FOE_BRINGUP_ERROR_NO_PHYSICAL_DEVICE_MEETS_REQUIREMENTS)

        // RenderGraph - RenderScene
        RESULT_CASE(FOE_BRINGUP_RENDER_SCENE_COLOUR_TARGET_NOT_IMAGE)
        RESULT_CASE(FOE_BRINGUP_RENDER_SCENE_COLOUR_TARGET_NOT_MUTABLE)
        RESULT_CASE(FOE_BRINGUP_RENDER_SCENE_COLOUR_TARGET_NO_STATE)
        RESULT_CASE(FOE_BRINGUP_RENDER_SCENE_DEPTH_TARGET_NOT_IMAGE)
        RESULT_CASE(FOE_BRINGUP_RENDER_SCENE_DEPTH_TARGET_NOT_MUTABLE)
        RESULT_CASE(FOE_BRINGUP_RENDER_SCENE_DEPTH_TARGET_NO_STATE)

        // Armature System
        RESULT_CASE(FOE_BRINGUP_ERROR_NO_ARMATURE_POOL_PROVIDED)
        RESULT_CASE(FOE_BRINGUP_ERROR_NO_ARMATURE_STATE_POOL_PROVIDED)

    default:
        if (value > 0) {
            sprintf(buffer, "FOE_BRINGUP_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            sprintf(buffer, "FOE_BRINGUP_UNKNOWN_ERROR_%i", value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}