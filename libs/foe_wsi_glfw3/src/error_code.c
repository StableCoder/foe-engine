// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/wsi/error_code.h>
#include <foe/wsi/export.h>

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
        RESULT_CASE(FOE_WSI_ERROR_FAILED_TO_INITIALIZE_BACKEND)
        RESULT_CASE(FOE_WSI_ERROR_FAILED_TO_CREATE_WINDOW)
        RESULT_CASE(FOE_WSI_ERROR_VULKAN_NOT_SUPPORTED)

    default:
        if (value > 0) {
            sprintf(buffer, "FOE_WSI_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            sprintf(buffer, "FOE_WSI_UNKNOWN_ERROR_%i", value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}