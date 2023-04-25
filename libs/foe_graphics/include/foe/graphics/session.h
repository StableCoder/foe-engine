// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_SESSION_H
#define FOE_GRAPHICS_SESSION_H

#include <foe/graphics/export.h>
#include <foe/handle.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeGfxSession)

FOE_GFX_EXPORT
void foeGfxDestroySession(foeGfxSession session);

/**
 * @brief Waits until the associated graphics session is idle
 * @param session Graphics session handle
 *
 * By 'idle' it is meant that the graphics session has no more commands to run or process.
 */
FOE_GFX_EXPORT
void foeGfxWaitIdle(foeGfxSession session);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_SESSION_H