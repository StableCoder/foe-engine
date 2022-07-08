// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "xr_result.h"

#define XR_RESULT_TO_STRING_CONFIG_MAIN
#include <xr_result_to_string.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

_Static_assert(XR_SUCCESS == FOE_SUCCESS, "XR_SUCCESS must equal FOE_SUCCESS");

void XrResultToString(XrResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = XrResult_to_string(value);

    if (pResultStr == NULL) {
        if (value > 0) {
            sprintf(buffer, "XR_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            sprintf(buffer, "XR_UNKNOWN_ERROR_%i", value);
        }
    }

    assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

    strcpy(buffer, pResultStr);
}
