// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_CLEANUP_H
#define FOE_GRAPHICS_VK_CLEANUP_H

#include <foe/graphics/export.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foeGfxVkShaderCreateInfo foeGfxVkShaderCreateInfo;

FOE_GFX_EXPORT
void cleanup_foeGfxVkShaderCreateInfo(foeGfxVkShaderCreateInfo *pData);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_VK_CLEANUP_H
