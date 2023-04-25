// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_COMPARE_H
#define FOE_GRAPHICS_VK_COMPARE_H

#include <foe/graphics/export.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foeGfxVkShaderCreateInfo foeGfxVkShaderCreateInfo;

FOE_GFX_EXPORT
bool compare_foeGfxVkShaderCreateInfo(foeGfxVkShaderCreateInfo const *pData1,
                                      foeGfxVkShaderCreateInfo const *pData2);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_VK_COMPARE_H
