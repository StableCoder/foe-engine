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
    FOE_IMGUI_VK_ERROR_OUT_OF_MEMORY = -1,
    FOE_IMGUI_VK_ERROR_GRAPH_UI_COLOUR_TARGET_NOT_IMAGE = -2,
    FOE_IMGUI_VK_ERROR_GRAPH_UI_COLOUR_TARGET_NOT_MUTABLE = -3,
    FOE_IMGUI_VK_ERROR_GRAPH_UI_COLOUR_TARGET_MISSING_STATE = -4,
} foeImGuiVkResult;

FOE_IMGUI_VK_EXPORT void foeImGuiVkResultToString(foeImGuiVkResult value,
                                                  char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_IMGUI_VK_RESULT_H