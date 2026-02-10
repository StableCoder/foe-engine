// Copyright (C) 2021-2023 George Cave.
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
    FOE_INCOMPLETE = 1000000001,
    FOE_AWAITING_INPUT = 1000000002,
    FOE_ERROR_OUT_OF_MEMORY = -1000000001,
    FOE_ERROR_FAILED_TO_OPEN_FILE = -1000000002,
    FOE_ERROR_FAILED_TO_STAT_FILE = -1000000003,
    FOE_ERROR_ATTEMPTED_TO_MAP_ZERO_SIZED_FILE = -1000000004,
    FOE_ERROR_FAILED_TO_MAP_FILE = -1000000005,
    FOE_ERROR_FAILED_TO_UNMAP_FILE = -1000000006,
    FOE_ERROR_FAILED_TO_CLOSE_FILE = -1000000007,
    FOE_ERROR_MEMORY_SUBSET_OVERRUNS_PARENT = -1000000008,
    FOE_ERROR_ZERO_SYNC_THREADS = -1000000009,
    FOE_ERROR_ZERO_ASYNC_THREADS = -1000000010,
    FOE_ERROR_DESTINATION_BUFFER_TOO_SMALL = -1000000011,
    FOE_ERROR_INVALID_HEX_DATA_SIZE = -1000000012,
    FOE_ERROR_MALFORMED_HEX_DATA = -1000000013,
    FOE_ERROR_UTF_MB_INCOMPLETE = -1000000014,
    FOE_ERROR_UTF_MALFORMED_DATA = -1000000015,
    FOE_ERROR_UTF_INVALID_STATE = -1000000016,
    FOE_ERROR_UTF_INVALID_CODEPOINT = -1000000017,
} foeResult;

FOE_EXPORT
void foeResultToString(foeResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_RESULT_H