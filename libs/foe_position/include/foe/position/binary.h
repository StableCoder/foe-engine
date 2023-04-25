// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_POSITION_BINARY_H
#define FOE_POSITION_BINARY_H

#include <foe/ecs/group_translator.h>
#include <foe/position/export.h>
#include <foe/result.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foePosition3d foePosition3d;

FOE_POSITION_EXPORT
foeResultSet binary_read_foePosition3d(void const *pReadBuffer,
                                       uint32_t *pReadSize,
                                       foePosition3d *pData);

FOE_POSITION_EXPORT
foeResultSet binary_write_foePosition3d(foePosition3d const *pData,
                                        uint32_t *pWriteSize,
                                        void *pWriteBuffer);

FOE_POSITION_EXPORT
char const *binary_key_foePosition3d();

#ifdef __cplusplus
}
#endif

#endif // FOE_POSITION_BINARY_H
