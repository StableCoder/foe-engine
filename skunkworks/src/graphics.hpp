// Copyright (C) 2021-2025 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <foe/graphics/runtime.h>
#include <foe/graphics/session.h>
#include <foe/result.h>
#include <foe/xr/runtime.h>
#include <vulkan/vulkan.h>

#include <string>
#include <vector>

foeResultSet createGfxRuntime(foeXrRuntime xrRuntime,
                              bool validation,
                              bool debugLogging,
                              std::vector<std::string> layers,
                              std::vector<std::string> extensions,
                              foeGfxRuntime *pGfxRuntime);

foeResultSet createGfxSession(foeGfxRuntime gfxRuntime,
                              foeXrRuntime xrRuntime,
                              bool enableWindowing,
                              std::vector<VkSurfaceKHR> windowSurfaces,
                              uint32_t explicitGpu,
                              bool forceXr,
                              foeGfxSession *pGfxSession);

#endif // GRAPHICS_HPP