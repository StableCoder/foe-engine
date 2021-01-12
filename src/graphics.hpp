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

#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <foe/graphics/runtime.hpp>
#include <foe/graphics/session.hpp>
#include <foe/xr/runtime.hpp>
#include <vulkan/vulkan.h>

#include <vector>
#include <system_error>

std::error_code createGfxRuntime(foeXrRuntime xrRuntime,
                                 bool enableWindowing,
                                 bool validation,
                                 bool debugLogging,
                                 foeGfxRuntime *pGfxRuntime);

std::error_code createGfxSession(foeGfxRuntime gfxRuntime,
                                 foeXrRuntime xrRuntime,
                                 bool enableWindowing,
                                 std::vector<VkSurfaceKHR> windowSurfaces,
                                 uint32_t explicitGpu,
                                 bool forceXr,
                                 foeGfxSession *pGfxSession);

#endif // GRAPHICS_HPP