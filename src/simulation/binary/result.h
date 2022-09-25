// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RESULT_H
#define RESULT_H

#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foeBringupBinaryResult {
    FOE_BRINGUP_BINARY_SUCCESS = 0,
    FOE_BRINGUP_BINARY_DATA_NOT_EXPORTED = 1,
    FOE_BRINGUP_BINARY_ERROR_UNSPECIFIED = -1,
    FOE_BRINGUP_BINARY_ERROR_OUT_OF_MEMORY = -2,
    FOE_BRINGUP_BINARY_ERROR_NO_EXPORTER = -3,
    FOE_BRINGUP_BINARY_ERROR_NO_CREATE_INFO_PROVIDED = -4,
    FOE_BRINGUP_BINARY_ERROR_FAILED_TO_REGISTER_ARMATURE_IMPORTER = -5,
    FOE_BRINGUP_BINARY_ERROR_ARMATURE_RESOURCE_ALREADY_EXISTS = -6,
    FOE_BRINGUP_BINARY_ERROR_FAILED_TO_REGISTER_ARMATURE_STATE_IMPORTER = -7,
    FOE_BRINGUP_BINARY_ERROR_ARMATURE_STATE_POOL_NOT_FOUND = -8,
    FOE_BRINGUP_BINARY_ERROR_FAILED_TO_REGISTER_CAMERA_IMPORTER = -9,
    FOE_BRINGUP_BINARY_ERROR_CAMERA_POOL_NOT_FOUND = -10,
    FOE_BRINGUP_BINARY_ERROR_FAILED_TO_REGISTER_RENDER_STATE_IMPORTER = -11,
    FOE_BRINGUP_BINARY_ERROR_RENDER_STATE_POOL_NOT_FOUND = -12,
} foeBringupBinaryResult;

void foeBringupBinaryResultToString(foeBringupBinaryResult value,
                                    char buffer[FOE_MAX_RESULT_STRING_SIZE]);

inline foeResultSet to_foeResult(foeBringupBinaryResult value) {
    foeResultSet result = {
        .value = value,
        .toString = (PFN_foeResultToString)foeBringupBinaryResultToString,
    };

    return result;
}

#ifdef __cplusplus
}
#endif

#endif // RESULT_H