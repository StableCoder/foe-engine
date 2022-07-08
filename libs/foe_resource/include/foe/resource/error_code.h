// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_RESOURCE_ERROR_CODE_H
#define FOE_RESOURCE_ERROR_CODE_H

#include <foe/error_code.h>
#include <foe/resource/export.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foeResourceResult {
    FOE_RESOURCE_SUCCESS = 0,
    FOE_RESOURCE_ERROR_NOT_FOUND,
    // General
    FOE_RESOURCE_ERROR_OUT_OF_MEMORY,
    // Records
    FOE_RESOURCE_CANNOT_UNDO,
    FOE_RESOURCE_CANNOT_REDO,
    FOE_RESOURCE_NO_MODIFIED_RECORD,
    FOE_RESOURCE_ERROR_EXISTING_RECORD,
    FOE_RESOURCE_ERROR_NON_EXISTING_RECORD,
    FOE_RESOURCE_ERROR_CANNOT_REMOVE_NON_PERSISTENT_RECORDS,
    FOE_RESOURCE_ERROR_NO_RECORDS,
    // Resource Specific
    FOE_RESOURCE_ERROR_RESOURCE_FUNCTIONS_NOT_PROVIDED,
    // CreateInfo Specific
    FOE_RESOURCE_ERROR_DATA_FUNCTION_NOT_PROVIDED,

    // Need to have a negative enum value to prevent treatment as a flag
    FOE_RESOURCE_ERROR_NEGATIVE_VALUE = FOE_RESULT_MIN_ENUM,
} foeResourceResult;

FOE_RES_EXPORT void foeResourceResultToString(foeResourceResult value,
                                              char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_RESOURCE_ERROR_CODE_H