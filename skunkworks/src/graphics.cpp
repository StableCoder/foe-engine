// Copyright (C) 2021-2025 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "graphics.hpp"

#include <foe/delimited_string.h>
#include <foe/graphics/vk/debug_callback.h>
#include <foe/graphics/vk/runtime.h>
#include <foe/graphics/vk/session.h>

#include "log.hpp"
#include "result.h"

#include <memory>
#include <vector>

#ifdef FOE_XR_SUPPORT
#include <foe/xr/openxr/runtime.h>
#include <foe/xr/openxr/vk/vulkan.h>
#endif

#ifndef VK_VERSION_1_2
static_assert(false, "FoE engine requires at least Vulkan 1.2 support (for timeline semaphores).");
#endif

namespace {

static VkDebugReportCallbackEXT registeredCallback = FOE_NULL_HANDLE;

VkBool32 vulkanMessageCallbacks(VkDebugReportFlagsEXT flags,
                                VkDebugReportObjectTypeEXT /*objectType*/,
                                uint64_t /*object*/,
                                size_t /*location*/,
                                int32_t messageCode,
                                char const *pLayerPrefix,
                                char const *pMessage,
                                void * /*pUserData*/) {
    if ((flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) != 0) {
        FOE_LOG(foeSkunkworks, FOE_LOG_LEVEL_ERROR, "[{}] Code {} : {}", pLayerPrefix, messageCode,
                pMessage)
    }
    if ((flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) != 0 ||
        (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) != 0) {
        FOE_LOG(foeSkunkworks, FOE_LOG_LEVEL_WARNING, "[{}] Code {} : {}", pLayerPrefix,
                messageCode, pMessage)
    }
    if ((flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) != 0) {
        FOE_LOG(foeSkunkworks, FOE_LOG_LEVEL_INFO, "[{}] Code {} : {}", pLayerPrefix, messageCode,
                pMessage)
    }
    if ((flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) != 0) {
        FOE_LOG(foeSkunkworks, FOE_LOG_LEVEL_VERBOSE, "[{}] Code {} : {}", pLayerPrefix,
                messageCode, pMessage)
    }

    return VK_FALSE;
}

} // namespace

foeResultSet createGfxRuntime(foeXrRuntime xrRuntime,
                              bool validation,
                              bool debugLogging,
                              std::vector<std::string> layers,
                              std::vector<std::string> extensions,
                              foeGfxRuntime *pGfxRuntime) {
#ifdef FOE_XR_SUPPORT
    // OpenXR
    if (xrRuntime != FOE_NULL_HANDLE) {
        uint32_t extensionsLength;
        foeResultSet result = foeXrGetVulkanInstanceExtensions(foeOpenXrGetInstance(xrRuntime),
                                                               &extensionsLength, nullptr);
        if (result.value != FOE_SUCCESS)
            return result;

        std::unique_ptr<char[]> extensionsStr{new char[extensionsLength]};

        result = foeXrGetVulkanInstanceExtensions(foeOpenXrGetInstance(xrRuntime),
                                                  &extensionsLength, extensionsStr.get());
        if (result.value != FOE_SUCCESS)
            return result;

        uint32_t strLength;
        char const *pStr;
        for (uint32_t i = 0; foeIndexedDelimitedString(extensionsLength, extensionsStr.get(), i,
                                                       ' ', &strLength, &pStr);
             ++i) {
            extensions.emplace_back(pStr, strLength);
        }

        // Add another that's missing??
        extensions.emplace_back("VK_KHR_external_fence_capabilities");
    }
#endif

    if (validation)
        layers.emplace_back("VK_LAYER_KHRONOS_validation");
    if (debugLogging)
        extensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

    // Always use the latest available runtime
    uint32_t vkApiVersion;
    vkEnumerateInstanceVersion(&vkApiVersion);

    std::vector<char const *> layersList;
    std::vector<char const *> extensionsList;

    for (auto &it : layers)
        layersList.emplace_back(it.data());
    for (auto &it : extensions)
        extensionsList.emplace_back(it.data());

    foeResultSet result =
        foeGfxVkCreateRuntime("FoE Engine", 0, vkApiVersion, layersList.size(), layersList.data(),
                              extensionsList.size(), extensionsList.data(), pGfxRuntime);

    if (debugLogging && result.value == FOE_SUCCESS) {
        result = foeGfxVkRegisterDebugCallback(
            *pGfxRuntime,
            VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_INFORMATION_BIT_EXT,
            &vulkanMessageCallbacks, &registeredCallback);
    }

    return result;
}

namespace {

auto determineVkPhysicalDevice(VkInstance vkInstance,
                               foeXrRuntime xrRuntime,
                               uint32_t surfaceCount,
                               VkSurfaceKHR *pSurfaces,
                               uint32_t explicitGpu,
                               bool forceXr) -> VkPhysicalDevice {
    // Just retrieves the first available device
    uint32_t physicalDeviceCount;
    VkResult vkRes = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, nullptr);
    if (vkRes != VK_SUCCESS || physicalDeviceCount == 0) {
        return VK_NULL_HANDLE;
    }

    std::unique_ptr<VkPhysicalDevice[]> physDevices(new VkPhysicalDevice[physicalDeviceCount]);
    vkRes = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, physDevices.get());
    if (vkRes != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }

    // OpenXR requirements
    VkPhysicalDevice xrPhysicalDevice{VK_NULL_HANDLE};
#ifdef FOE_XR_SUPPORT
    if (xrRuntime != FOE_NULL_HANDLE) {
        foeXrGetVulkanGraphicsDevice(foeOpenXrGetInstance(xrRuntime), 0, vkInstance,
                                     &xrPhysicalDevice);
    }
#endif

    // Window Requirements
    std::vector<VkBool32> supportsWindow(physicalDeviceCount, VK_FALSE);

    for (uint32_t i = 0; i < physicalDeviceCount; ++i) {
        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(physDevices[i], &queueFamilyCount, nullptr);

        uint32_t supportedSurfaces = 0;
        for (uint32_t j = 0; j < surfaceCount; ++j) {
            for (uint32_t queueIndex = 0; queueIndex < queueFamilyCount; ++queueIndex) {
                VkBool32 usable;
                vkGetPhysicalDeviceSurfaceSupportKHR(physDevices[i], queueIndex, pSurfaces[j],
                                                     &usable);

                if (usable == VK_TRUE) {
                    ++supportedSurfaces;
                    break;
                }
            }
        }
        if (supportedSurfaces == surfaceCount) {
            supportsWindow[i] = VK_TRUE;
        }
    }

    // If we've been given an explicit GPU...
    if (explicitGpu != UINT32_MAX) {
        if (explicitGpu < physicalDeviceCount) {
            // It exists, spit out warning if it doesn't support OpenXR or windowing
            if (xrPhysicalDevice != VK_NULL_HANDLE &&
                xrPhysicalDevice != physDevices[explicitGpu]) {
                FOE_LOG(foeSkunkworks, FOE_LOG_LEVEL_WARNING,
                        "Explicit Physical GPU specified, OpenXR is possible but not supported on "
                        "this GPU: {}",
                        explicitGpu < physicalDeviceCount)
            }
            if (supportsWindow[explicitGpu] == VK_FALSE) {
                FOE_LOG(foeSkunkworks, FOE_LOG_LEVEL_WARNING,
                        "Explicit Physical GPU specified, Windowing is not supported on this "
                        "GPU: {}",
                        explicitGpu < physicalDeviceCount)
            }

            return physDevices[explicitGpu];
        } else {
            // Invalid device index given
            FOE_LOG(foeSkunkworks, FOE_LOG_LEVEL_ERROR,
                    "Explicit Physical GPU specified, but could not be found: {}", explicitGpu);
        }
    }

    // First, try one that supports both OpenXR and Windowing
    for (uint32_t i = 0; i < physicalDeviceCount; ++i) {
        if (supportsWindow[i] == VK_TRUE && physDevices[i] == xrPhysicalDevice) {
            return xrPhysicalDevice;
        }
    }

    FOE_LOG(foeSkunkworks, FOE_LOG_LEVEL_VERBOSE,
            "Failed to find device that supported both XR and windowing")

    // Second, try a XR-only device (if specified)
    if (forceXr) {
        if (xrPhysicalDevice != VK_NULL_HANDLE) {
            FOE_LOG(foeSkunkworks, FOE_LOG_LEVEL_INFO,
                    "Using a VkPhysicalDevice that *only* supports XR");
            return xrPhysicalDevice;
        } else {
            FOE_LOG(foeSkunkworks, FOE_LOG_LEVEL_ERROR,
                    "Attempted to use XR, however no physical device supports it currently");
        }
    }

    // Thirdly, try a Window-only capable device
    for (uint32_t i = 0; i < physicalDeviceCount; ++i) {
        if (supportsWindow[i] == VK_TRUE) {
            FOE_LOG(foeSkunkworks, FOE_LOG_LEVEL_INFO,
                    "Using VkPhysicalDevice that only supports windowing")
            return physDevices[i];
        }
    }

    if (surfaceCount == 0) {
        FOE_LOG(foeSkunkworks, FOE_LOG_LEVEL_INFO,
                "Using VkPhysicalDevice that doesn't necessarily support XR or Windowing");
        return physDevices[0];
    }

    FOE_LOG(foeSkunkworks, FOE_LOG_LEVEL_ERROR,
            "Failed to find physical device that fit any requirements");
    return VK_NULL_HANDLE;
}

} // namespace

foeResultSet createGfxSession(foeGfxRuntime gfxRuntime,
                              foeXrRuntime xrRuntime,
                              bool enableWindowing,
                              std::vector<VkSurfaceKHR> windowSurfaces,
                              uint32_t explicitGpu,
                              bool forceXr,
                              foeGfxSession *pGfxSession) {
    // Determine the physical device
    VkPhysicalDevice vkPhysicalDevice = determineVkPhysicalDevice(
        foeGfxVkGetRuntimeInstance(gfxRuntime), xrRuntime, windowSurfaces.size(),
        windowSurfaces.data(), explicitGpu, forceXr);
    if (vkPhysicalDevice == VK_NULL_HANDLE)
        return to_foeResult(FOE_SKUNKWORKS_ERROR_NO_PHYSICAL_DEVICE_MEETS_REQUIREMENTS);

    // Layers and Extensions
    std::vector<std::string> layers;
    std::vector<std::string> extensions;

    if (enableWindowing) {
        extensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

#ifdef FOE_XR_SUPPORT
    // OpenXR
    if (xrRuntime != FOE_NULL_HANDLE) {
        uint32_t extensionsLength;
        foeResultSet result = foeXrGetVulkanDeviceExtensions(foeOpenXrGetInstance(xrRuntime),
                                                             &extensionsLength, nullptr);
        if (result.value != FOE_SUCCESS)
            return result;

        std::unique_ptr<char[]> extensionsStr{new char[extensionsLength]};

        result = foeXrGetVulkanDeviceExtensions(foeOpenXrGetInstance(xrRuntime), &extensionsLength,
                                                extensionsStr.get());
        if (result.value != FOE_SUCCESS)
            return result;

        uint32_t strLength;
        char const *pStr;
        for (uint32_t i = 0; foeIndexedDelimitedString(extensionsLength, extensionsStr.get(), i,
                                                       ' ', &strLength, &pStr);
             ++i) {
            extensions.emplace_back(pStr, strLength);
        }
    }
#endif

    std::vector<char const *> layersList;
    std::vector<char const *> extensionsList;

    for (auto &it : layers)
        layersList.emplace_back(it.data());
    for (auto &it : extensions)
        extensionsList.emplace_back(it.data());

    // Get full list of available device extensions
    uint32_t deviceExtensionCount;
    vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &deviceExtensionCount, nullptr);
    std::vector<VkExtensionProperties> extProps{deviceExtensionCount};
    vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &deviceExtensionCount,
                                         extProps.data());

    // Check support for timeline semaphores/synchronization2
    bool synchronization2 = false;
    bool timelineSemaphores = false;

    for (uint32_t i = 0; i < extProps.size(); ++i) {
        if (strcmp(extProps[i].extensionName, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME) == 0) {
            synchronization2 = true;
            break;
        } else if (strcmp(extProps[i].extensionName, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME) ==
                   0) {
            timelineSemaphores = true;
        }
    }

    if (synchronization2) {
        extensionsList.emplace_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    } else if (timelineSemaphores) {
        extensionsList.emplace_back(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    } else {
        return to_foeResult(FOE_SKUNKWORKS_ERROR_NO_TIMELINE_SEMAPHORE_SUPPORT);
    }

    return foeGfxVkCreateSession(gfxRuntime, vkPhysicalDevice, layersList.size(), layersList.data(),
                                 extensionsList.size(), extensionsList.data(), nullptr, nullptr,
                                 pGfxSession);
}