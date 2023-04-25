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
    FOE_WSI_ERROR_OUT_OF_MEMORY = -1000003001,
    FOE_WSI_ERROR_FAILED_TO_INITIALIZE_BACKEND = -1000003002,
    FOE_WSI_ERROR_FAILED_TO_CREATE_WINDOW = -1000003003,
    FOE_WSI_ERROR_VULKAN_NOT_SUPPORTED = -1000003004,
} foeWsiResult;

FOE_WSI_EXPORT
void foeWsiResultToString(foeWsiResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_WSI_RESULT_H