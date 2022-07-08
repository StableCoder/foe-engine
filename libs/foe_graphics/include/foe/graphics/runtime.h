// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RUNTIME_H
#define FOE_GRAPHICS_RUNTIME_H

#include <foe/graphics/export.h>
#include <foe/handle.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeGfxRuntime)

FOE_GFX_EXPORT void foeGfxDestroyRuntime(foeGfxRuntime runtime);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_RUNTIME_H