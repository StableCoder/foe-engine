// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_RESOURCE_RESULT_H
#define FOE_RESOURCE_RESULT_H

#include <foe/resource/export.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foeResourceResult {
    FOE_RESOURCE_SUCCESS = 0,
    FOE_RESOURCE_NO_MODIFIED_RECORD = 1000007003,
    FOE_RESOURCE_ALREADY_LOADING = 1000007004,
    FOE_RESOURCE_ERROR_NOT_FOUND = -1000007001,
    FOE_RESOURCE_ERROR_OUT_OF_MEMORY = -1000007002,
    FOE_RESOURCE_ERROR_RESOURCE_FUNCTIONS_NOT_PROVIDED = -1000007003,
    FOE_RESOURCE_ERROR_DATA_FUNCTION_NOT_PROVIDED = -1000007004,
    FOE_RESOURCE_ERROR_DATA_SIZE_SMALLER_THAN_BASE = -1000007005,
    FOE_RESOURCE_ERROR_RESOURCE_NOT_UNDEFINED = -1000007006,
    FOE_RESOURCE_ERROR_REPLACED_CANNOT_BE_LOADED = -1000007007,
} foeResourceResult;

FOE_RES_EXPORT void foeResourceResultToString(foeResourceResult value,
                                              char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_RESOURCE_RESULT_H