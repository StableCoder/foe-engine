// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "result.h"

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
        RESULT_CASE(FOE_BRINGUP_ERROR_OUT_OF_MEMORY)
        RESULT_CASE(FOE_BRINGUP_INITIALIZATION_FAILED)
        RESULT_CASE(FOE_BRINGUP_NOT_INITIALIZED)
        RESULT_CASE(FOE_BRINGUP_FAILED_TO_LOAD_PLUGIN)
        RESULT_CASE(FOE_BRINGUP_ERROR_NO_TIMELINE_SEMAPHORE_SUPPORT)
        RESULT_CASE(FOE_BRINGUP_ERROR_LOADER_INITIALIZATION_FAILED)
        RESULT_CASE(FOE_BRINGUP_ERROR_INCOMPATIBLE_CREATE_INFO)
        RESULT_CASE(FOE_BRINGUP_ERROR_INCOMPATIBLE_RESOURCE)
        RESULT_CASE(FOE_BRINGUP_ERROR_IMPORT_FAILED)
        RESULT_CASE(FOE_BRINGUP_ERROR_FAILED_TO_REGISTER_EXPORTERS)
        RESULT_CASE(FOE_BRINGUP_ERROR_NO_PHYSICAL_DEVICE_MEETS_REQUIREMENTS)
        RESULT_CASE(FOE_BRINGUP_RENDER_SCENE_COLOUR_TARGET_NOT_IMAGE)
        RESULT_CASE(FOE_BRINGUP_RENDER_SCENE_COLOUR_TARGET_NOT_MUTABLE)
        RESULT_CASE(FOE_BRINGUP_RENDER_SCENE_COLOUR_TARGET_NO_STATE)
        RESULT_CASE(FOE_BRINGUP_RENDER_SCENE_DEPTH_TARGET_NOT_IMAGE)
        RESULT_CASE(FOE_BRINGUP_RENDER_SCENE_DEPTH_TARGET_NOT_MUTABLE)
        RESULT_CASE(FOE_BRINGUP_RENDER_SCENE_DEPTH_TARGET_NO_STATE)
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
