// Copyright (C) 2022-2024 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/simulation/result.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        pResultStr = #X;                                                                           \
        break;

void foeSimulationResultToString(foeSimulationResult value,
                                 char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = NULL;

    switch (value) {
        RESULT_CASE(FOE_SIMULATION_SUCCESS)
        RESULT_CASE(FOE_SIMULATION_CANNOT_UNDO)
        RESULT_CASE(FOE_SIMULATION_CANNOT_REDO)
        RESULT_CASE(FOE_SIMULATION_ERROR_OUT_OF_MEMORY)
        RESULT_CASE(FOE_SIMULATION_ERROR_NOT_REGISTERED)
        RESULT_CASE(FOE_SIMULATION_ERROR_NO_LOADER_FOUND)
        RESULT_CASE(FOE_SIMULATION_ERROR_ID_INVALID)
        RESULT_CASE(FOE_SIMULATION_ERROR_ID_ALREADY_IN_USE)
        RESULT_CASE(FOE_SIMULATION_ERROR_TYPE_NOT_FOUND)
        RESULT_CASE(FOE_SIMULATION_ERROR_TYPE_ALREADY_EXISTS)
        RESULT_CASE(FOE_SIMULATION_ERROR_CREATING_RESOURCE_POOL)
        RESULT_CASE(FOE_SIMULATION_ERROR_SIMULATION_ALREADY_INITIALIZED)
        RESULT_CASE(FOE_SIMULATION_ERROR_SIMULATION_NOT_INITIALIZED)
        RESULT_CASE(FOE_SIMULATION_ERROR_SIMULATION_GRAPHICS_ALREADY_INITIALIZED)
        RESULT_CASE(FOE_SIMULATION_ERROR_SIMULATION_GRAPHICS_NOT_INITIALIZED)
        RESULT_CASE(FOE_SIMULATION_ERROR_NO_CREATE_INFO)
        RESULT_CASE(FOE_SIMULATION_ERROR_RESOURCE_CREATE_INFO_ALREADY_ADDED)
        RESULT_CASE(FOE_SIMULATION_ERROR_RESOURCE_CREATE_INFO_NOT_FOUND)

    default:
        if (value > 0) {
            snprintf(buffer, FOE_MAX_RESULT_STRING_SIZE, "FOE_SIMULATION_UNKNOWN_SUCCESS_%i",
                     value);
        } else {
            value = abs(value);
            snprintf(buffer, FOE_MAX_RESULT_STRING_SIZE, "FOE_SIMULATION_UNKNOWN_ERROR_%i", value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}
