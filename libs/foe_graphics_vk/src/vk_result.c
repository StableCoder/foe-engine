// Copyright (C) 2022-2024 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "vk_result.h"

#define VK_RESULT_TO_STRING_CONFIG_MAIN
#include <vk_result_to_string.h>

#include <assert.h>
#include <string.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

_Static_assert(VK_SUCCESS == FOE_SUCCESS, "VK_SUCCESS must equal FOE_SUCCESS");

void VkResultToString(VkResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = VkResult_to_string(value);

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    } else {
        if (value > 0) {
            snprintf(buffer, FOE_MAX_RESULT_STRING_SIZE, "VK_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            snprintf(buffer, FOE_MAX_RESULT_STRING_SIZE, "VK_UNKNOWN_ERROR_%i", value);
        }
    }
}
