// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/imex/binary/result.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        pResultStr = #X;                                                                           \
        break;

void foeImexBinaryResultToString(foeImexBinaryResult value,
                                 char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = NULL;

    switch (value) {
        RESULT_CASE(FOE_IMEX_BINARY_SUCCESS)
        RESULT_CASE(FOE_IMEX_BINARY_INCOMPLETE)
        RESULT_CASE(FOE_IMEX_BINARY_ERROR_OUT_OF_MEMORY)
        RESULT_CASE(FOE_IMEX_BINARY_ERROR_FUNCTIONALITY_ALREADY_REGISTERED)
        RESULT_CASE(FOE_IMEX_BINARY_ERROR_FUNCTIONALITY_NOT_REGISTERED)
        RESULT_CASE(FOE_IMEX_BINARY_ERROR_DESTINATION_NOT_FILE)
        RESULT_CASE(FOE_IMEX_BINARY_ERROR_FAILED_TO_EXPORT_DEPENDENCIES)
        RESULT_CASE(FOE_IMEX_BINARY_ERROR_FAILED_TO_EXPORT_RESOURCE)
        RESULT_CASE(FOE_IMEX_BINARY_ERROR_KEY_ALREADY_REGISTERED)
        RESULT_CASE(FOE_IMEX_BINARY_ERROR_KEY_NOT_REGISTERED)
        RESULT_CASE(FOE_IMEX_BINARY_ERROR_KEY_FUNCTIONS_NON_MATCHING)
        RESULT_CASE(FOE_IMEX_BINARY_ERROR_FILE_NOT_EXIST)
        RESULT_CASE(FOE_IMEX_BINARY_ERROR_NOT_REGULAR_FILE)
        RESULT_CASE(FOE_IMEX_BINARY_ERROR_FAILED_TO_OPEN_FILE)
        RESULT_CASE(FOE_IMEX_BINARY_ERROR_FAILED_TO_FIND_RESOURCE_DATA)
        RESULT_CASE(FOE_IMEX_BINARY_ERROR_FAILED_TO_FIND_EXTERNAL_DATA)

    default:
        if (value > 0) {
            sprintf(buffer, "FOE_IMEX_BINARY_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            sprintf(buffer, "FOE_IMEX_BINARY_UNKNOWN_ERROR_%i", value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}
