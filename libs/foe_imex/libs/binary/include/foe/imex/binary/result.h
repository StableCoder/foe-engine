// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_IMEX_BINARY_RESULT_H
#define FOE_IMEX_BINARY_RESULT_H

#include <foe/imex/binary/export.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foeImexBinaryResult {
    FOE_IMEX_BINARY_SUCCESS = 0,
    FOE_IMEX_BINARY_INCOMPLETE = 1,
    FOE_IMEX_BINARY_ERROR_OUT_OF_MEMORY = -1,
    FOE_IMEX_BINARY_ERROR_FUNCTIONALITY_ALREADY_REGISTERED = -2,
    FOE_IMEX_BINARY_ERROR_FUNCTIONALITY_NOT_REGISTERED = -3,
    FOE_IMEX_BINARY_ERROR_DESTINATION_NOT_FILE = -4,
    FOE_IMEX_BINARY_ERROR_FAILED_TO_EXPORT_DEPENDENCIES = -5,
    FOE_IMEX_BINARY_ERROR_FAILED_TO_EXPORT_RESOURCE = -6,
    FOE_IMEX_BINARY_ERROR_KEY_ALREADY_REGISTERED = -7,
    FOE_IMEX_BINARY_ERROR_KEY_NOT_REGISTERED = -8,
    FOE_IMEX_BINARY_ERROR_KEY_FUNCTIONS_NON_MATCHING = -9,
    FOE_IMEX_BINARY_ERROR_FILE_NOT_EXIST = -10,
    FOE_IMEX_BINARY_ERROR_NOT_REGULAR_FILE = -11,
} foeImexBinaryResult;

FOE_IMEX_BINARY_EXPORT void foeImexBinaryResultToString(foeImexBinaryResult value,
                                                        char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_IMEX_BINARY_RESULT_H