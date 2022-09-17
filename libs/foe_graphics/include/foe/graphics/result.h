// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RESULT_H
#define FOE_GRAPHICS_RESULT_H

#include <foe/graphics/export.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foeGraphicsResult {
    FOE_GRAPHICS_SUCCESS = 0,
    FOE_GRAPHICS_ERROR_OUT_OF_MEMORY = -1,
} foeGraphicsResult;

FOE_GFX_EXPORT void foeGraphicsResultToString(foeGraphicsResult value,
                                              char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_RESULT_H