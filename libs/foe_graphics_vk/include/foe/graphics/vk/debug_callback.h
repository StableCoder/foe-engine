// Copyright (C) 2025 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_DEBUG_CALLBACK_H
#define FOE_GRAPHICS_VK_DEBUG_CALLBACK_H

#include <foe/graphics/export.h>
#include <foe/graphics/runtime.h>
#include <foe/result.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_GFX_EXPORT
foeResultSet foeGfxVkRegisterDebugCallback(foeGfxRuntime runtime,
                                           VkDebugReportFlagsEXT flags,
                                           PFN_vkDebugReportCallbackEXT pfnCallback,
                                           VkDebugReportCallbackEXT *pDebugCallback);

FOE_GFX_EXPORT
foeResultSet foeGfxVkDeregisterDebugCallback(foeGfxRuntime runtime,
                                             VkDebugReportCallbackEXT debugCallback);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_VK_DEBUG_CALLBACK_H