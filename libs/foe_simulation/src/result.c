// Copyright (C) 2022-2023 George Cave.
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

    default:
        if (value > 0) {
            sprintf(buffer, "FOE_SIMULATION_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            sprintf(buffer, "FOE_SIMULATION_UNKNOWN_ERROR_%i", value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}
