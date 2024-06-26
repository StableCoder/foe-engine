// Copyright (C) 2022-2024 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/imgui/vk/result.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        pResultStr = #X;                                                                           \
        break;

void foeImGuiVkResultToString(foeImGuiVkResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = NULL;

    switch (value) {
        RESULT_CASE(FOE_IMGUI_VK_SUCCESS)
        RESULT_CASE(FOE_IMGUI_VK_ERROR_OUT_OF_MEMORY)
        RESULT_CASE(FOE_IMGUI_VK_ERROR_GRAPH_UI_COLOUR_TARGET_NOT_IMAGE)
        RESULT_CASE(FOE_IMGUI_VK_ERROR_GRAPH_UI_COLOUR_TARGET_NOT_MUTABLE)
        RESULT_CASE(FOE_IMGUI_VK_ERROR_GRAPH_UI_COLOUR_TARGET_MISSING_STATE)

    default:
        if (value > 0) {
            snprintf(buffer, FOE_MAX_RESULT_STRING_SIZE, "FOE_IM_GUI_VK_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            snprintf(buffer, FOE_MAX_RESULT_STRING_SIZE, "FOE_IM_GUI_VK_UNKNOWN_ERROR_%i", value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}
