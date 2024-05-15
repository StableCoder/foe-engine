// Copyright (C) 2022-2024 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/resource/result.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        pResultStr = #X;                                                                           \
        break;

void foeResourceResultToString(foeResourceResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = NULL;

    switch (value) {
        RESULT_CASE(FOE_RESOURCE_SUCCESS)
        RESULT_CASE(FOE_RESOURCE_NO_MODIFIED_RECORD)
        RESULT_CASE(FOE_RESOURCE_ALREADY_LOADING)
        RESULT_CASE(FOE_RESOURCE_ERROR_NOT_FOUND)
        RESULT_CASE(FOE_RESOURCE_ERROR_OUT_OF_MEMORY)
        RESULT_CASE(FOE_RESOURCE_ERROR_RESOURCE_FUNCTIONS_NOT_PROVIDED)
        RESULT_CASE(FOE_RESOURCE_ERROR_DATA_FUNCTION_NOT_PROVIDED)
        RESULT_CASE(FOE_RESOURCE_ERROR_DATA_SIZE_SMALLER_THAN_BASE)
        RESULT_CASE(FOE_RESOURCE_ERROR_RESOURCE_NOT_UNDEFINED)
        RESULT_CASE(FOE_RESOURCE_ERROR_REPLACED_CANNOT_BE_LOADED)

    default:
        if (value > 0) {
            snprintf(buffer, FOE_MAX_RESULT_STRING_SIZE, "FOE_RESOURCE_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            snprintf(buffer, FOE_MAX_RESULT_STRING_SIZE, "FOE_RESOURCE_UNKNOWN_ERROR_%i", value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}
