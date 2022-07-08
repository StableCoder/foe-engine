// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RENDER_TARGET_H
#define FOE_GRAPHICS_RENDER_TARGET_H

#include <foe/error_code.h>
#include <foe/graphics/export.h>
#include <foe/handle.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeGfxRenderTarget)

FOE_GFX_EXPORT void foeGfxDestroyRenderTarget(foeGfxRenderTarget renderTarget);

FOE_GFX_EXPORT void foeGfxUpdateRenderTargetExtent(foeGfxRenderTarget renderTarget,
                                                   uint32_t width,
                                                   uint32_t height);

FOE_GFX_EXPORT foeResult foeGfxAcquireNextRenderTarget(foeGfxRenderTarget renderTarget,
                                                       uint32_t maxBufferedFrames);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_RENDER_TARGET_H