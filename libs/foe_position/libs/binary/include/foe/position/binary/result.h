// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_POSITION_BINARY_RESULT_H
#define FOE_POSITION_BINARY_RESULT_H

#include <foe/position/binary/export.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foePositionBinaryResult {
    FOE_POSITION_BINARY_SUCCESS = 0,
    FOE_POSITION_BINARY_DATA_NOT_EXPORTED = 1000017001,
    FOE_POSITION_BINARY_ERROR_OUT_OF_MEMORY = -1000017001,
    FOE_POSITION_BINARY_ERROR_FAILED_TO_REGISTER_3D_IMPORTER = -1000017002,
    FOE_POSITION_BINARY_ERROR_FAILED_TO_REGISTER_3D_EXPORTER = -1000017003,
    FOE_POSITION_BINARY_ERROR_POSITION_3D_POOL_NOT_FOUND = -1000017004,
} foePositionBinaryResult;

FOE_POSITION_BINARY_EXPORT
void foePositionBinaryResultToString(foePositionBinaryResult value,
                                     char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_POSITION_BINARY_RESULT_H