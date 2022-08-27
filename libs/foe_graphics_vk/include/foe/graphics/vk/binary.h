// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_BINARY_H
#define FOE_GRAPHICS_VK_BINARY_H

#include <foe/graphics/export.h>
#include <foe/result.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foeGfxVkShaderCreateInfo foeGfxVkShaderCreateInfo;

FOE_GFX_EXPORT foeResultSet binary_read_foeGfxVkShaderCreateInfo(void const *pReadBuffer,
                                                                 uint32_t *pReadSize,
                                                                 foeGfxVkShaderCreateInfo *pData);

FOE_GFX_EXPORT foeResultSet binary_write_foeGfxVkShaderCreateInfo(
    foeGfxVkShaderCreateInfo const *pData, uint32_t *pWriteSize, void *pWriteBuffer);

FOE_GFX_EXPORT char const *binary_key_foeGfxVkShaderCreateInfo();

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_VK_BINARY_H
