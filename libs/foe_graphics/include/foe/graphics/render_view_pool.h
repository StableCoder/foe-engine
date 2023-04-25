// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RENDER_VIEW_POOL_H
#define FOE_GRAPHICS_RENDER_VIEW_POOL_H

#include <foe/graphics/export.h>
#include <foe/graphics/session.h>
#include <foe/handle.h>
#include <foe/result.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeGfxRenderViewPool)
FOE_DEFINE_HANDLE(foeGfxRenderView)

FOE_GFX_EXPORT
foeResultSet foeGfxCreateRenderViewPool(foeGfxSession session,
                                        uint32_t viewCount,
                                        foeGfxRenderViewPool *pRenderViewPool);

FOE_GFX_EXPORT
void foeGfxDestroyRenderViewPool(foeGfxSession session, foeGfxRenderViewPool renderViewPool);

FOE_GFX_EXPORT
foeResultSet foeGfxUpdateRenderViewPool(foeGfxSession session, foeGfxRenderViewPool renderViewPool);

FOE_GFX_EXPORT
foeResultSet foeGfxAllocateRenderView(foeGfxRenderViewPool renderViewPool,
                                      foeGfxRenderView *pRenderView);

FOE_GFX_EXPORT
void foeGfxFreeRenderView(foeGfxRenderView renderView);

FOE_GFX_EXPORT
foeResultSet foeGfxUpdateRenderView(foeGfxRenderView renderView, uint32_t dataSize, void *pData);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_RENDER_VIEW_POOL_H