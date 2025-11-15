// Copyright (C) 2021-2025 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/debug_callback.h>
#include <foe/graphics/vk/runtime.h>

#include "result.h"
#include "vk_result.h"

foeResultSet foeGfxVkRegisterDebugCallback(foeGfxRuntime runtime,
                                           VkDebugReportFlagsEXT flags,
                                           PFN_vkDebugReportCallbackEXT pfnCallback,
                                           VkDebugReportCallbackEXT *pDebugCallback) {
    VkDebugReportCallbackCreateInfoEXT debugCallbackCI{
        .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
        .flags = flags,
        .pfnCallback = pfnCallback,
    };

    VkInstance vkInstance = foeGfxVkGetRuntimeInstance(runtime);

    auto fpCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
        vkGetInstanceProcAddr(vkInstance, "vkCreateDebugReportCallbackEXT"));
    if (fpCreateDebugReportCallbackEXT == nullptr) {
        return vk_to_foeResult(VK_ERROR_INITIALIZATION_FAILED);
    }

    VkResult result =
        fpCreateDebugReportCallbackEXT(vkInstance, &debugCallbackCI, nullptr, pDebugCallback);
    return vk_to_foeResult(result);
}

foeResultSet foeGfxVkDeregisterDebugCallback(foeGfxRuntime runtime,
                                             VkDebugReportCallbackEXT debugCallback) {
    VkInstance vkInstance = foeGfxVkGetRuntimeInstance(runtime);

    auto fpDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
        vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugReportCallbackEXT"));
    if (fpDestroyDebugReportCallbackEXT == nullptr) {
        return vk_to_foeResult(VK_ERROR_INITIALIZATION_FAILED);
    }

    fpDestroyDebugReportCallbackEXT(vkInstance, debugCallback, nullptr);

    return to_foeResult(FOE_GRAPHICS_VK_SUCCESS);
}