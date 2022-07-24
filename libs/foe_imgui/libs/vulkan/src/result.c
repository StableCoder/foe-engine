// Copyright (C) 2022 George Cave.
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
        // RenderGraph - UI Job
        RESULT_CASE(FOE_IMGUI_VK_GRAPH_UI_COLOUR_TARGET_NOT_IMAGE)
        RESULT_CASE(FOE_IMGUI_VK_GRAPH_UI_COLOUR_TARGET_NOT_MUTABLE)
        RESULT_CASE(FOE_IMGUI_VK_GRAPH_UI_COLOUR_TARGET_MISSING_STATE)

    default:
        if (value > 0) {
            sprintf(buffer, "FOE_IMGUI_VK_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            sprintf(buffer, "FOE_IMGUI_VK_UNKNOWN_ERROR_%i", value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}