// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_IMEX_RESULT_H
#define FOE_IMEX_RESULT_H

#include <foe/imex/export.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foeImexResult {
    FOE_IMEX_SUCCESS = 0,
    FOE_IMEX_ERROR_INCOMPLETE = 1,
    FOE_IMEX_ERROR_FUNCTION_NOT_DEFINED = -1,
    FOE_IMEX_ERROR_STRUCTURE_NOT_FOUND = -2,
    FOE_IMEX_ERROR_FUNCTIONALITY_ALREADY_REGISTERED = -3,
    FOE_IMEX_ERROR_EXPORTER_ALREADY_REGISTERED = -4,
    FOE_IMEX_ERROR_EXPORTER_NOT_REGISTERED = -5,
    FOE_IMEX_ERROR_IMPORTER_ALREADY_REGISTERED = -6,
    FOE_IMEX_ERROR_IMPORTER_NOT_REGISTERED = -7,
} foeImexResult;

FOE_IMEX_EXPORT void foeImexResultToString(foeImexResult value,
                                           char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_IMEX_RESULT_H