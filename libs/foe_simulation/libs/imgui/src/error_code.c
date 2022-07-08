// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/simulation/imgui/error_code.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        pResultStr = #X;                                                                           \
        break;

void foeSimulationImGuiResultToString(foeSimulationImGuiResult value,
                                      char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = NULL;

    switch (value) {
        RESULT_CASE(FOE_SIMULATION_IMGUI_SUCCESS)
        RESULT_CASE(FOE_SIMULATION_IMGUI_ERROR_ALL_PARAMETERS_NULL)
        RESULT_CASE(FOE_SIMULATION_IMGUI_ERROR_FUNCTIONALITY_ALREADY_REGISTERED)
        RESULT_CASE(FOE_SIMULATION_IMGUI_ERROR_FUNCTIONALITY_NOT_REGISTERED)

    default:
        if (value > 0) {
            sprintf(buffer, "FOE_SIMULATION_IMGUI_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            sprintf(buffer, "FOE_SIMULATION_IMGUI_UNKNOWN_ERROR_%i", value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}