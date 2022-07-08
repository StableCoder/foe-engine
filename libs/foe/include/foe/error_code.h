// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_ERROR_CODE_H
#define FOE_ERROR_CODE_H

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

typedef struct foeResult {
    int value;
    PFN_foeResultToString toString;
} foeResult;

#ifdef __cplusplus
}
#endif

#endif // FOE_ERROR_CODE_H