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

#ifndef FOE_GRAPHICS_DELAYED_DESTRUCTOR_HPP
#define FOE_GRAPHICS_DELAYED_DESTRUCTOR_HPP

#include <foe/graphics/export.h>
#include <foe/graphics/session.hpp>
#include <foe/handle.h>

#include <functional>
#include <system_error>

FOE_DEFINE_HANDLE(foeGfxDelayedDestructor)

using foeGfxDelayedDestructorFn = std::function<void(foeGfxSession)>;

FOE_GFX_EXPORT auto foeGfxCreateDelayedDestructor(foeGfxSession session,
                                                  uint32_t initialDelay,
                                                  foeGfxDelayedDestructor *pDelayedDestructor)
    -> std::error_code;

FOE_GFX_EXPORT void foeGfxDestroyDelayedDestructor(foeGfxDelayedDestructor delayedDestructor);

/// Advances to the next set of delayed destruction calls and runs them. Meant to be synchronized to
/// the frames.
FOE_GFX_EXPORT void foeGfxRunDelayedDestructor(foeGfxDelayedDestructor delayedDestructor);

FOE_GFX_EXPORT void foeGfxAddDelayedDestructionCall(foeGfxDelayedDestructor delayedDestructor,
                                                    foeGfxDelayedDestructorFn fn);

FOE_GFX_EXPORT void foeGfxAddDelayedDestructionCall(foeGfxDelayedDestructor delayedDestructor,
                                                    foeGfxDelayedDestructorFn fn,
                                                    uint32_t numDelayed);

#endif // FOE_GRAPHICS_DELAYED_DESTRUCTOR_HPP