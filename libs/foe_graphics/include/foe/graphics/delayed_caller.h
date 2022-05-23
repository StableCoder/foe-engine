/*
    Copyright (C) 2021-2022 George Cave.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef FOE_GRAPHICS_DELAYED_CALLER_H
#define FOE_GRAPHICS_DELAYED_CALLER_H

#include <foe/error_code.h>
#include <foe/graphics/export.h>
#include <foe/graphics/session.h>
#include <foe/handle.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeGfxDelayedCaller)

typedef void (*PFN_foeGfxDelayedCall)(void *, foeGfxSession);

FOE_GFX_EXPORT foeResult foeGfxCreateDelayedCaller(foeGfxSession session,
                                                   uint32_t initialDelay,
                                                   foeGfxDelayedCaller *pDelayedCaller);

FOE_GFX_EXPORT void foeGfxDestroyDelayedCaller(foeGfxDelayedCaller delayedCaller);

/// Advances to the next set of delayed destruction calls and runs them. Meant to be synchronized to
/// the frames.
FOE_GFX_EXPORT void foeGfxRunDelayedCalls(foeGfxDelayedCaller delayedCaller);

FOE_GFX_EXPORT void foeGfxAddDefaultDelayedCall(foeGfxDelayedCaller delayedCaller,
                                                PFN_foeGfxDelayedCall destroyCall,
                                                void *pContext);

FOE_GFX_EXPORT void foeGfxAddDelayedCall(foeGfxDelayedCaller delayedCaller,
                                         PFN_foeGfxDelayedCall destroyCall,
                                         void *pContext,
                                         uint32_t numDelayed);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_DELAYED_CALLER_H