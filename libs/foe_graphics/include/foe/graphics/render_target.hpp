/*
    Copyright (C) 2021 George Cave.

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

#ifndef FOE_GRAPHICS_RENDER_TARGET_HPP
#define FOE_GRAPHICS_RENDER_TARGET_HPP

#include <foe/graphics/export.h>
#include <foe/handle.h>

#include <system_error>

FOE_DEFINE_HANDLE(foeGfxRenderTarget)

FOE_GFX_EXPORT void foeGfxDestroyRenderTarget(foeGfxRenderTarget renderTarget);

FOE_GFX_EXPORT void foeGfxUpdateRenderTargetExtent(foeGfxRenderTarget renderTarget,
                                                   uint32_t width,
                                                   uint32_t height);

FOE_GFX_EXPORT std::error_code foeGfxAcquireNextRenderTarget(foeGfxRenderTarget renderTarget,
                                                             uint32_t maxBufferedFrames);

#endif // FOE_GRAPHICS_RENDER_TARGET_HPP