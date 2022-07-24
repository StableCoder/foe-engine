// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_WSI_RESULT_H
#define FOE_WSI_RESULT_H

#include <foe/result.h>
#include <foe/wsi/export.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foeWsiResult {
    FOE_WSI_SUCCESS = 0,
    FOE_WSI_ERROR_FAILED_TO_INITIALIZE_BACKEND,
    FOE_WSI_ERROR_FAILED_TO_CREATE_WINDOW,
    FOE_WSI_ERROR_VULKAN_NOT_SUPPORTED,

    // Need to have a negative enum value to prevent treatment as a flag
    FOE_WSI_ERROR_NEGATIVE_VALUE = FOE_RESULT_MIN_ENUM,
} foeWsiResult;

FOE_WSI_EXPORT void foeWsiResultToString(foeWsiResult value,
                                         char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_WSI_RESULT_H