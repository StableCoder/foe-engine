// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RESULT_H
#define RESULT_H

#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foeBringupResult {
    FOE_BRINGUP_SUCCESS = 0,

    FOE_BRINGUP_ERROR_OUT_OF_MEMORY,
    FOE_BRINGUP_INITIALIZATION_FAILED,
    FOE_BRINGUP_NOT_INITIALIZED,
    FOE_BRINGUP_FAILED_TO_LOAD_PLUGIN,

    FOE_BRINGUP_ERROR_NO_TIMELINE_SEMAPHORE_SUPPORT,

    // Loaders
    FOE_BRINGUP_ERROR_LOADER_INITIALIZATION_FAILED,
    FOE_BRINGUP_ERROR_INCOMPATIBLE_CREATE_INFO,
    FOE_BRINGUP_ERROR_INCOMPATIBLE_RESOURCE,
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

inline foeResultSet to_foeResult(foeBringupResult value) {
    foeResultSet result = {
        .value = value,
        .toString = (PFN_foeResultToString)foeBringupResultToString,
    };

    return result;
}

#ifdef __cplusplus
}
#endif

#endif // RESULT_H