// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_XR_OPENXR_VK_RESULT_H
#define FOE_XR_OPENXR_VK_RESULT_H

#include <foe/result.h>
#include <foe/xr/export.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foeOpenXrVkResult {
    FOE_OPENXR_VK_SUCCESS = 0,
    FOE_OPENXR_VK_ERROR_OUT_OF_MEMORY = -1000013001,
} foeOpenXrVkResult;

FOE_XR_EXPORT
void foeOpenXrVkResultToString(foeOpenXrVkResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_XR_OPENXR_VK_RESULT_H