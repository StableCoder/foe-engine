// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <foe/graphics/runtime.h>
#include <foe/graphics/session.h>
#include <foe/result.h>
#include <foe/xr/runtime.h>
#include <vulkan/vulkan.h>

#include <vector>

foeResultSet createGfxRuntime(foeXrRuntime xrRuntime,
                              bool enableWindowing,
                              bool validation,
                              bool debugLogging,
                              foeGfxRuntime *pGfxRuntime);

foeResultSet createGfxSession(foeGfxRuntime gfxRuntime,
                              foeXrRuntime xrRuntime,
                              bool enableWindowing,
                              std::vector<VkSurfaceKHR> windowSurfaces,
                              uint32_t explicitGpu,
                              bool forceXr,
                              foeGfxSession *pGfxSession);

#endif // GRAPHICS_HPP