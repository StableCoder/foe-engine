// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_XR_OPENXR_RESULT_H
#define FOE_XR_OPENXR_RESULT_H

#include <foe/result.h>
#include <foe/xr/export.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foeOpenXrResult {
    FOE_OPENXR_SUCCESS = 0,
    FOE_OPENXR_INCOMPLETE = 1000012001,
    FOE_OPENXR_ERROR_OUT_OF_MEMORY = -1000012001,
} foeOpenXrResult;

FOE_XR_EXPORT
void foeOpenXrResultToString(foeOpenXrResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_XR_OPENXR_RESULT_H