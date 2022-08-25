// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_RESULT_H
#define FOE_RESULT_H

#include <foe/export.h>

#ifdef __cplusplus
extern "C" {
#endif

/// The minimum possible result code (not quite the full rangle to allow conversion to the absolute
/// value)
#define FOE_RESULT_MIN_ENUM -0x7FFFFFFF
/// The maximum possible result code
#define FOE_RESULT_MAX_ENUM 0x7FFFFFFF
/// The maximum length that a result code name that can be returned from a
/// <result>ToString(<result>) function
#define FOE_MAX_RESULT_STRING_SIZE 128

/// This is the form of the function that can convert a given Result value and turn it into a
/// human-readable string
typedef void (*PFN_foeResultToString)(int value, char buffer[FOE_MAX_RESULT_STRING_SIZE]);

typedef struct foeResultSet {
    int value;
    PFN_foeResultToString toString;
} foeResultSet;

typedef enum foeResult {
    FOE_SUCCESS = 0,
    FOE_ERROR_OUT_OF_MEMORY = -1,
    FOE_ERROR_FAILED_TO_OPEN_FILE = -2,
    FOE_ERROR_FAILED_TO_STAT_FILE = -3,
    FOE_ERROR_ATTEMPTED_TO_MAP_ZERO_SIZED_FILE = -4,
    FOE_ERROR_FAILED_TO_MAP_FILE = -5,
    FOE_ERROR_FAILED_TO_UNMAP_FILE = -6,
    FOE_ERROR_FAILED_TO_CLOSE_FILE = -7,
    FOE_ERROR_MEMORY_SUBSET_OVERRUNS_PARENT = -8,
} foeResult;

FOE_EXPORT void foeResultToString(foeResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_RESULT_H