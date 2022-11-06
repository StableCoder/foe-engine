// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/result.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        pResultStr = #X;                                                                           \
        break;

void foeResultToString(foeResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = NULL;

    switch (value) {
        RESULT_CASE(FOE_SUCCESS)
        RESULT_CASE(FOE_ERROR_OUT_OF_MEMORY)
        RESULT_CASE(FOE_ERROR_FAILED_TO_OPEN_FILE)
        RESULT_CASE(FOE_ERROR_FAILED_TO_STAT_FILE)
        RESULT_CASE(FOE_ERROR_ATTEMPTED_TO_MAP_ZERO_SIZED_FILE)
        RESULT_CASE(FOE_ERROR_FAILED_TO_MAP_FILE)
        RESULT_CASE(FOE_ERROR_FAILED_TO_UNMAP_FILE)
        RESULT_CASE(FOE_ERROR_FAILED_TO_CLOSE_FILE)
        RESULT_CASE(FOE_ERROR_MEMORY_SUBSET_OVERRUNS_PARENT)
        RESULT_CASE(FOE_ERROR_ZERO_SYNC_THREADS)
        RESULT_CASE(FOE_ERROR_ZERO_ASYNC_THREADS)

    default:
        if (value > 0) {
            sprintf(buffer, "FOE_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            sprintf(buffer, "FOE_UNKNOWN_ERROR_%i", value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}
