// Copyright (C) 2024 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/position/binary/result.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        pResultStr = #X;                                                                           \
        break;

void foePositionBinaryResultToString(foePositionBinaryResult value,
                                     char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = NULL;

    switch (value) {
        RESULT_CASE(FOE_POSITION_BINARY_SUCCESS)
        RESULT_CASE(FOE_POSITION_BINARY_DATA_NOT_EXPORTED)
        RESULT_CASE(FOE_POSITION_BINARY_ERROR_OUT_OF_MEMORY)
        RESULT_CASE(FOE_POSITION_BINARY_ERROR_FAILED_TO_REGISTER_3D_IMPORTER)
        RESULT_CASE(FOE_POSITION_BINARY_ERROR_FAILED_TO_REGISTER_3D_EXPORTER)
        RESULT_CASE(FOE_POSITION_BINARY_ERROR_POSITION_3D_POOL_NOT_FOUND)

    default:
        if (value > 0) {
            snprintf(buffer, FOE_MAX_RESULT_STRING_SIZE, "FOE_POSITION_BINARY_UNKNOWN_SUCCESS_%i",
                     value);
        } else {
            value = abs(value);
            snprintf(buffer, FOE_MAX_RESULT_STRING_SIZE, "FOE_POSITION_BINARY_UNKNOWN_ERROR_%i",
                     value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}
