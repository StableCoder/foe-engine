// Copyright (C) 2021-2023 George Cave.
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
    FOE_PHYSICS_ERROR_OUT_OF_MEMORY = -1000018001,
    FOE_PHYSICS_ERROR_COLLISION_SHAPE_LOADER_INITIALIZATION_FAILED = -1000018002,
    FOE_PHYSICS_ERROR_INCOMPATIBLE_CREATE_INFO = -1000018003,
    FOE_PHYSICS_ERROR_INCOMPATIBLE_RESOURCE_TYPE = -1000018004,
    FOE_PHYSICS_ERROR_MISSING_COLLISION_SHAPE_RESOURCES = -1000018006,
    FOE_PHYSICS_ERROR_MISSING_RIGID_BODY_COMPONENTS = -1000018007,
    FOE_PHYSICS_ERROR_MISSING_POSITION_3D_COMPONENTS = -1000018008,
} foePhysicsResult;

FOE_PHYSICS_EXPORT void foePhysicsResultToString(foePhysicsResult value,
                                                 char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_PHYSICS_RESULT_H