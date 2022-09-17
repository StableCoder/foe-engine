// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_POSITION_RESULT_H
#define FOE_POSITION_RESULT_H

#include <foe/position/export.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foePositionResult {
    FOE_POSITION_SUCCESS = 0,
    FOE_POSITION_ERROR_OUT_OF_MEMORY = -1,
} foePositionResult;

FOE_POSITION_EXPORT void foePositionResultToString(foePositionResult value,
                                                   char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_POSITION_RESULT_H