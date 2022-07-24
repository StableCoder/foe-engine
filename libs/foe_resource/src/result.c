// Copyright (C) 2022 George Cave.
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
        RESULT_CASE(FOE_RESOURCE_ERROR_NOT_FOUND)
        // General
        RESULT_CASE(FOE_RESOURCE_ERROR_OUT_OF_MEMORY)
        // Records
        RESULT_CASE(FOE_RESOURCE_CANNOT_UNDO)
        RESULT_CASE(FOE_RESOURCE_CANNOT_REDO)
        RESULT_CASE(FOE_RESOURCE_NO_MODIFIED_RECORD)
        RESULT_CASE(FOE_RESOURCE_ERROR_EXISTING_RECORD)
        RESULT_CASE(FOE_RESOURCE_ERROR_NON_EXISTING_RECORD)
        RESULT_CASE(FOE_RESOURCE_ERROR_CANNOT_REMOVE_NON_PERSISTENT_RECORDS)
        RESULT_CASE(FOE_RESOURCE_ERROR_NO_RECORDS)
        // Resource Specific
        RESULT_CASE(FOE_RESOURCE_ERROR_RESOURCE_FUNCTIONS_NOT_PROVIDED)
        // CreateInfo Specific
        RESULT_CASE(FOE_RESOURCE_ERROR_DATA_FUNCTION_NOT_PROVIDED)

    default:
        if (value > 0) {
            sprintf(buffer, "FOE_RESOURCE_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            sprintf(buffer, "FOE_RESOURCE_UNKNOWN_ERROR_%i", value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}