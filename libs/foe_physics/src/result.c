// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/physics/result.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        pResultStr = #X;                                                                           \
        break;

void foePhysicsResultToString(foePhysicsResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = NULL;

    switch (value) {
        RESULT_CASE(FOE_PHYSICS_SUCCESS)
        RESULT_CASE(FOE_PHYSICS_ERROR_OUT_OF_MEMORY)
        RESULT_CASE(FOE_PHYSICS_ERROR_COLLISION_SHAPE_LOADER_INITIALIZATION_FAILED)
        RESULT_CASE(FOE_PHYSICS_ERROR_INCOMPATIBLE_CREATE_INFO)
        RESULT_CASE(FOE_PHYSICS_ERROR_INCOMPATIBLE_RESOURCE_TYPE)
        RESULT_CASE(FOE_PHYSICS_ERROR_MISSING_COLLISION_SHAPE_RESOURCES)
        RESULT_CASE(FOE_PHYSICS_ERROR_MISSING_RIGID_BODY_COMPONENTS)
        RESULT_CASE(FOE_PHYSICS_ERROR_MISSING_POSITION_3D_COMPONENTS)

    default:
        if (value > 0) {
            sprintf(buffer, "FOE_PHYSICS_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            sprintf(buffer, "FOE_PHYSICS_UNKNOWN_ERROR_%i", value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}
