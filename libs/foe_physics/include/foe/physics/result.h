// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_PHYSICS_RESULT_H
#define FOE_PHYSICS_RESULT_H

#include <foe/physics/export.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foePhysicsResult {
    FOE_PHYSICS_SUCCESS = 0,
    FOE_PHYSICS_ERROR_OUT_OF_MEMORY,
    // Loaders
    FOE_PHYSICS_ERROR_COLLISION_SHAPE_LOADER_INITIALIZATION_FAILED,
    FOE_PHYSICS_ERROR_INCOMPATIBLE_CREATE_INFO,
    FOE_PHYSICS_ERROR_INCOMPATIBLE_RESOURCE_TYPE,
    // Physics System
    FOE_PHYSICS_ERROR_MISSING_COLLISION_SHAPE_LOADER,
    FOE_PHYSICS_ERROR_MISSING_COLLISION_SHAPE_RESOURCES,
    FOE_PHYSICS_ERROR_MISSING_RIGID_BODY_COMPONENTS,
    FOE_PHYSICS_ERROR_MISSING_POSITION_3D_COMPONENTS,

    // Need to have a negative enum value to prevent treatment as a flag
    FOE_PHYSICS_ERROR_NEGATIVE_VALUE = FOE_RESULT_MIN_ENUM,
} foePhysicsResult;

FOE_PHYSICS_EXPORT void foePhysicsResultToString(foePhysicsResult value,
                                                 char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_PHYSICS_RESULT_H