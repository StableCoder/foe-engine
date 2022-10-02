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
    FOE_IMEX_ERROR_INCOMPLETE = 1000002001,
    FOE_IMEX_ERROR_FUNCTION_NOT_DEFINED = -1000002001,
    FOE_IMEX_ERROR_STRUCTURE_NOT_FOUND = -1000002002,
    FOE_IMEX_ERROR_FUNCTIONALITY_ALREADY_REGISTERED = -1000002003,
    FOE_IMEX_ERROR_EXPORTER_ALREADY_REGISTERED = -1000002004,
    FOE_IMEX_ERROR_EXPORTER_NOT_REGISTERED = -1000002005,
    FOE_IMEX_ERROR_IMPORTER_ALREADY_REGISTERED = -1000002006,
    FOE_IMEX_ERROR_IMPORTER_NOT_REGISTERED = -1000002007,
} foeImexResult;

FOE_IMEX_EXPORT void foeImexResultToString(foeImexResult value,
                                           char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_IMEX_RESULT_H