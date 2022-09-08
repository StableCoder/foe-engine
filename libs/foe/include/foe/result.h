// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_RESULT_H
#define FOE_RESULT_H

#ifdef __cplusplus
extern "C" {
#endif

/// Global value for an unqualified 'SUCCESS' result code
#define FOE_SUCCESS 0
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
    FOE_ERROR_OUT_OF_MEMORY = -10000,
    FOE_ERROR_FAILED_TO_OPEN_FILE,
    FOE_ERROR_FAILED_TO_STAT_FILE,
    FOE_ERROR_ATTEMPTED_TO_MAP_ZERO_SIZED_FILE,
    FOE_ERROR_FAILED_TO_MAP_FILE,
    FOE_ERROR_FAILED_TO_UNMAP_FILE,
    FOE_ERROR_FAILED_TO_CLOSE_FILE,
} foeResult;

void foeResultToString(foeResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_RESULT_H