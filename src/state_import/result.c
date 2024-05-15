// Copyright (C) 2022-2024 George Cave.
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

void foeStateImportResultToString(foeStateImportResult value,
                                  char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = NULL;

    switch (value) {
        RESULT_CASE(FOE_STATE_IMPORT_SUCCESS)
        RESULT_CASE(FOE_STATE_IMPORT_ERROR_NO_IMPORTER)
        RESULT_CASE(FOE_STATE_IMPORT_ERROR_IMPORTING_DEPENDENCIES)
        RESULT_CASE(FOE_STATE_IMPORT_ERROR_DUPLICATE_DEPENDENCIES)
        RESULT_CASE(FOE_STATE_IMPORT_ERROR_TRANSITIVE_DEPENDENCIES_UNFULFILLED)
        RESULT_CASE(FOE_STATE_IMPORT_ERROR_ECS_GROUP_SETUP_FAILURE)
        RESULT_CASE(FOE_STATE_IMPORT_ERROR_IMPORTING_INDEX_DATA)
        RESULT_CASE(FOE_STATE_IMPORT_ERROR_IMPORTING_RESOURCE)
        RESULT_CASE(FOE_STATE_IMPORT_ERROR_NO_COMPONENT_IMPORTER)
    default:
        if (value > 0) {
            snprintf(buffer, FOE_MAX_RESULT_STRING_SIZE, "FOE_STATE_IMPORT_UNKNOWN_SUCCESS_%i",
                     value);
        } else {
            value = abs(value);
            snprintf(buffer, FOE_MAX_RESULT_STRING_SIZE, "FOE_STATE_IMPORT_UNKNOWN_ERROR_%i",
                     value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}