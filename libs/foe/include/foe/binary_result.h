// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_BINARY_RESULT_H
#define FOE_BINARY_RESULT_H

#include <foe/export.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foeBinaryResult {
    FOE_BINARY_SUCCESS = 0,
    FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE,
    FOE_BINARY_ERROR_OUT_OF_MEMORY,

    // Need to have a negative enum value to prevent treatment as a flag
    FOE_BINARY_RESULT_NEGATIVE_VALUE = FOE_RESULT_MIN_ENUM,
} foeBinaryResult;

FOE_EXPORT
void foeBinaryResultToString(foeBinaryResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]);

inline static foeResultSet foeBinaryResult_to_foeResultSet(foeBinaryResult value) {
    foeResultSet result = {
        .value = value,
        .toString = (PFN_foeResultToString)foeBinaryResultToString,
    };

    return result;
}

#ifdef __cplusplus
}
#endif

#endif // FOE_BINARY_RESULT_H