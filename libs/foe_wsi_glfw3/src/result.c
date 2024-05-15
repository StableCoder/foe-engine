// Copyright (C) 2022-2024 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/wsi/export.h>
#include <foe/wsi/result.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        pResultStr = #X;                                                                           \
        break;

void foeWsiResultToString(foeWsiResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = NULL;

    switch (value) {
        RESULT_CASE(FOE_WSI_SUCCESS)
        RESULT_CASE(FOE_WSI_ERROR_OUT_OF_MEMORY)
        RESULT_CASE(FOE_WSI_ERROR_FAILED_TO_INITIALIZE_BACKEND)
        RESULT_CASE(FOE_WSI_ERROR_FAILED_TO_CREATE_WINDOW)
        RESULT_CASE(FOE_WSI_ERROR_VULKAN_NOT_SUPPORTED)

    default:
        if (value > 0) {
            snprintf(buffer, FOE_MAX_RESULT_STRING_SIZE, "FOE_WSI_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            snprintf(buffer, FOE_MAX_RESULT_STRING_SIZE, "FOE_WSI_UNKNOWN_ERROR_%i", value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}