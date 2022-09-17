// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/split_thread_pool.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        pResultStr = #X;                                                                           \
        break;

void foeSplitThreadResultToString(foeSplitThreadResult value,
                                  char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = NULL;

    switch (value) {
        RESULT_CASE(FOE_THREAD_POOL_SUCCESS)
        RESULT_CASE(FOE_THREAD_POOL_ERROR_OUT_OF_MEMORY)
        RESULT_CASE(FOE_THREAD_POOL_ERROR_ZERO_SYNC_THREADS)
        RESULT_CASE(FOE_THREAD_POOL_ERROR_ZERO_ASYNC_THREADS)
        RESULT_CASE(FOE_THREAD_POOL_ERROR_ALLOCATION_FAILED)
        RESULT_CASE(FOE_THREAD_POOL_ERROR_ALREADY_STARTED)
        RESULT_CASE(FOE_THREAD_POOL_ERROR_NOT_STARTED)

    default:
        if (value > 0) {
            sprintf(buffer, "FOE_THREAD_POOL_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            sprintf(buffer, "FOE_THREAD_POOL_UNKNOWN_ERROR_%i", value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}