// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_XR_OPENXR_ERROR_CODE_H
#define FOE_XR_OPENXR_ERROR_CODE_H

#include <foe/error_code.h>
#include <foe/xr/export.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foeOpenXrResult {
    FOE_OPENXR_SUCCESS = 0,
    FOE_OPENXR_INCOMPLETE,

    // Need to have a negative enum value to prevent treatment as a flag
    FOE_OPENXR_ERROR_NEGATIVE_VALUE = FOE_RESULT_MIN_ENUM,
} foeOpenXrResult;

FOE_XR_EXPORT void foeOpenXrResultToString(foeOpenXrResult value,
                                           char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_XR_OPENXR_ERROR_CODE_H