// Copyright (C) 2022 George Cave.
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
    FOE_RESOURCE_CANNOT_UNDO = 1,
    FOE_RESOURCE_CANNOT_REDO = 2,
    FOE_RESOURCE_NO_MODIFIED_RECORD = 3,
    FOE_RESOURCE_ERROR_NOT_FOUND = -1,
    FOE_RESOURCE_ERROR_OUT_OF_MEMORY = -2,
    FOE_RESOURCE_ERROR_EXISTING_RECORD = -3,
    FOE_RESOURCE_ERROR_NON_EXISTING_RECORD = -4,
    FOE_RESOURCE_ERROR_CANNOT_REMOVE_NON_PERSISTENT_RECORDS = -5,
    FOE_RESOURCE_ERROR_NO_RECORDS = -6,
    FOE_RESOURCE_ERROR_NO_CREATE_INFO = -7,
    FOE_RESOURCE_ERROR_RESOURCE_FUNCTIONS_NOT_PROVIDED = -8,
    FOE_RESOURCE_ERROR_DATA_FUNCTION_NOT_PROVIDED = -9,
} foeResourceResult;

FOE_RES_EXPORT void foeResourceResultToString(foeResourceResult value,
                                              char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_RESOURCE_RESULT_H