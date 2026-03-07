// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_HEX_H
#define FOE_HEX_H

#include <foe/export.h>
#include <foe/result.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_EXPORT
foeResultSet foeEncodeHex(size_t dataSize,
                          void const *pData,
                          size_t hexBufferSize,
                          char *pHexBuffer);

FOE_EXPORT
foeResultSet foeDecodeHex(size_t hexDataSize, char const *pHexData, size_t *pDataSize, void *pData);

#ifdef __cplusplus
}
#endif

#endif // FOE_HEX_H