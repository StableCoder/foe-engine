// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "debug_callback.hpp"

#include "log.hpp"

namespace {

VkBool32 vulkanMessageCallbacks(VkDebugReportFlagsEXT flags,
                                VkDebugReportObjectTypeEXT /*objectType*/,
                                uint64_t /*object*/,
                                size_t /*location*/,
                                int32_t messageCode,
                                const char *pLayerPrefix,
                                const char *pMessage,
                                void * /*pUserData*/) {
    if ((flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) != 0) {
        FOE_LOG(foeVkGraphics, Error, "[{}] Code {} : {}", pLayerPrefix, messageCode, pMessage)
    }
    if ((flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) != 0 ||
        (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) != 0) {
        FOE_LOG(foeVkGraphics, Warning, "[{}] Code {} : {}", pLayerPrefix, messageCode, pMessage)
    }
    if ((flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) != 0) {
        FOE_LOG(foeVkGraphics, Info, "[{}] Code {} : {}", pLayerPrefix, messageCode, pMessage)
    }
    if ((flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) != 0) {
        FOE_LOG(foeVkGraphics, Verbose, "[{}] Code {} : {}", pLayerPrefix, messageCode, pMessage)
    }

    return VK_FALSE;
}

} // namespace

VkResult foeVkCreateDebugCallback(VkInstance instance, VkDebugReportCallbackEXT *pDebugCallback) {
    VkDebugReportCallbackCreateInfoEXT debugCallbackCI{
        .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
        .flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                 VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_INFORMATION_BIT_EXT,
        .pfnCallback = &vulkanMessageCallbacks,
    };

    auto fpCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
        vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
    if (fpCreateDebugReportCallbackEXT == nullptr) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    return fpCreateDebugReportCallbackEXT(instance, &debugCallbackCI, nullptr, pDebugCallback);
}

VkResult foeVkDestroyDebugCallback(VkInstance instance, VkDebugReportCallbackEXT debugCallback) {
    auto fpDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
        vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
    if (fpDestroyDebugReportCallbackEXT == nullptr) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    fpDestroyDebugReportCallbackEXT(instance, debugCallback, nullptr);

    return VK_SUCCESS;
}