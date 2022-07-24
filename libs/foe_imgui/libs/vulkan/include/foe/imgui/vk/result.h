// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_IMGUI_VK_RESULT_H
#define FOE_IMGUI_VK_RESULT_H

#include <foe/imgui/vk/export.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foeImGuiVkResult {
    FOE_IMGUI_VK_SUCCESS = 0,
    // RenderGraph - UI Job
    FOE_IMGUI_VK_GRAPH_UI_COLOUR_TARGET_NOT_IMAGE,
    FOE_IMGUI_VK_GRAPH_UI_COLOUR_TARGET_NOT_MUTABLE,
    FOE_IMGUI_VK_GRAPH_UI_COLOUR_TARGET_MISSING_STATE,

    // Need to have a negative enum value to prevent treatment as a flag
    FOE_IMGUI_VK_ERROR_NEGATIVE_VALUE = FOE_RESULT_MIN_ENUM,
} foeImGuiVkResult;

FOE_IMGUI_VK_EXPORT void foeImGuiVkResultToString(foeImGuiVkResult value,
                                                  char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_IMGUI_VK_RESULT_H