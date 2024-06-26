// Copyright (C) 2022-2024 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/xr/openxr/result.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        pResultStr = #X;                                                                           \
        break;

void foeOpenXrResultToString(foeOpenXrResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = NULL;

    switch (value) {
        RESULT_CASE(FOE_OPENXR_SUCCESS)
        RESULT_CASE(FOE_OPENXR_INCOMPLETE)
        RESULT_CASE(FOE_OPENXR_ERROR_OUT_OF_MEMORY)

    default:
        if (value > 0) {
            snprintf(buffer, FOE_MAX_RESULT_STRING_SIZE, "FOE_OPEN_XR_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            snprintf(buffer, FOE_MAX_RESULT_STRING_SIZE, "FOE_OPEN_XR_UNKNOWN_ERROR_%i", value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}
