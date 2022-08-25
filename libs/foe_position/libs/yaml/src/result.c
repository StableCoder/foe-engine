// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/position/yaml/result.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        pResultStr = #X;                                                                           \
        break;

void foePositionYamlResultToString(foePositionYamlResult value,
                                   char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = NULL;

    switch (value) {
        RESULT_CASE(FOE_POSITION_YAML_SUCCESS)
        RESULT_CASE(FOE_POSITION_YAML_ERROR_FAILED_TO_REGISTER_3D_IMPORTER)
        RESULT_CASE(FOE_POSITION_YAML_ERROR_FAILED_TO_REGISTER_3D_EXPORTER)

    default:
        if (value > 0) {
            sprintf(buffer, "FOE_POSITION_YAML_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            sprintf(buffer, "FOE_POSITION_YAML_UNKNOWN_ERROR_%i", value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}
