// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "result.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        pResultStr = #X;                                                                           \
        break;

void foeBringupBinaryResultToString(foeBringupBinaryResult value,
                                    char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = NULL;

    switch (value) {
        RESULT_CASE(FOE_BRINGUP_BINARY_SUCCESS)
        RESULT_CASE(FOE_BRINGUP_BINARY_DATA_NOT_EXPORTED)
        RESULT_CASE(FOE_BRINGUP_BINARY_ERROR_UNSPECIFIED)
        RESULT_CASE(FOE_BRINGUP_BINARY_ERROR_OUT_OF_MEMORY)
        RESULT_CASE(FOE_BRINGUP_BINARY_ERROR_ARMATURE_POOL_NOT_FOUND)
        RESULT_CASE(FOE_BRINGUP_BINARY_ERROR_ARMATURE_RESOURCE_ALREADY_EXISTS)
        RESULT_CASE(FOE_BRINGUP_BINARY_ERROR_FAILED_TO_REGISTER_ARMATURE_IMPORTER)
        RESULT_CASE(FOE_BRINGUP_BINARY_ERROR_FAILED_TO_REGISTER_ARMATURE_STATE_IMPORTER)
        RESULT_CASE(FOE_BRINGUP_BINARY_ERROR_FAILED_TO_REGISTER_RENDER_STATE_IMPORTER)
        RESULT_CASE(FOE_BRINGUP_BINARY_ERROR_FAILED_TO_REGISTER_CAMERA_IMPORTER)
        RESULT_CASE(FOE_BRINGUP_BINARY_ERROR_NO_CREATE_INFO_PROVIDED)
        RESULT_CASE(FOE_BRINGUP_BINARY_ERROR_NO_EXPORTER)
        RESULT_CASE(FOE_BRINGUP_BINARY_ERROR_FAILED_TO_REGISTER_RESOURCE_EXPORTERS)
        RESULT_CASE(FOE_BRINGUP_BINARY_ERROR_FAILED_TO_REGISTER_COMPONENT_EXPORTERS)

    default:
        if (value > 0) {
            sprintf(buffer, "FOE_BRINGUP_BINARY_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            sprintf(buffer, "FOE_BRINGUP_BINARY_UNKNOWN_ERROR_%i", value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}
