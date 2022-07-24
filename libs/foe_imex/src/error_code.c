// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/imex/error_code.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        pResultStr = #X;                                                                           \
        break;

void foeImexResultToString(foeImexResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = NULL;

    switch (value) {
        RESULT_CASE(FOE_IMEX_SUCCESS)
        RESULT_CASE(FOE_IMEX_ERROR_INCOMPLETE)
        RESULT_CASE(FOE_IMEX_ERROR_FUNCTION_NOT_DEFINED)
        RESULT_CASE(FOE_IMEX_ERROR_STRUCTURE_NOT_FOUND)
        RESULT_CASE(FOE_IMEX_ERROR_FUNCTIONALITY_ALREADY_REGISTERED)
        // Exporter
        RESULT_CASE(FOE_IMEX_ERROR_EXPORTER_ALREADY_REGISTERED)
        RESULT_CASE(FOE_IMEX_ERROR_EXPORTER_NOT_REGISTERED)
        // Importer
        RESULT_CASE(FOE_IMEX_ERROR_IMPORTER_ALREADY_REGISTERED)
        RESULT_CASE(FOE_IMEX_ERROR_IMPORTER_NOT_REGISTERED)

    default:
        if (value > 0) {
            sprintf(buffer, "FOE_IMEX_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            sprintf(buffer, "FOE_IMEX_UNKNOWN_ERROR_%i", value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}