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
    FOE_IMEX_BINARY_INCOMPLETE = 1000005001,
    FOE_IMEX_BINARY_ERROR_OUT_OF_MEMORY = -1000005001,
    FOE_IMEX_BINARY_ERROR_FUNCTIONALITY_ALREADY_REGISTERED = -1000005002,
    FOE_IMEX_BINARY_ERROR_FUNCTIONALITY_NOT_REGISTERED = -1000005003,
    FOE_IMEX_BINARY_ERROR_DESTINATION_NOT_FILE = -1000005004,
    FOE_IMEX_BINARY_ERROR_FAILED_TO_EXPORT_DEPENDENCIES = -1000005005,
    FOE_IMEX_BINARY_ERROR_FAILED_TO_EXPORT_RESOURCE = -1000005006,
    FOE_IMEX_BINARY_ERROR_KEY_ALREADY_REGISTERED = -1000005007,
    FOE_IMEX_BINARY_ERROR_KEY_NOT_REGISTERED = -1000005008,
    FOE_IMEX_BINARY_ERROR_KEY_FUNCTIONS_NON_MATCHING = -1000005009,
    FOE_IMEX_BINARY_ERROR_FILE_NOT_EXIST = -1000005010,
    FOE_IMEX_BINARY_ERROR_NOT_REGULAR_FILE = -1000005011,
    FOE_IMEX_BINARY_ERROR_FAILED_TO_OPEN_FILE = -1000005012,
    FOE_IMEX_BINARY_ERROR_FAILED_TO_FIND_RESOURCE_DATA = -1000005013,
    FOE_IMEX_BINARY_ERROR_FAILED_TO_FIND_EXTERNAL_DATA = -1000005014,
} foeImexBinaryResult;

FOE_IMEX_BINARY_EXPORT
void foeImexBinaryResultToString(foeImexBinaryResult value,
                                 char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_IMEX_BINARY_RESULT_H