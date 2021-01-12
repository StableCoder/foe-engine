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

#include "graphics.hpp"

#include <foe/graphics/vk/runtime.hpp>
#include <foe/graphics/vk/session.hpp>
#include <foe/log.hpp>
#include <foe/wsi_vulkan.hpp>
#include <vk_error_code.hpp>

#include <memory>

#ifdef FOE_XR_SUPPORT
#include <foe/xr/openxr/runtime.hpp>
#include <foe/xr/vulkan.hpp>
#endif

std::error_code createGfxRuntime(foeXrRuntime xrRuntime,
                                 bool enableWindowing,
                                 bool validation,
                                 bool debugLogging,
                                 foeGfxRuntime *pGfxRuntime) {
    std::vector<std::string> layers;
    std::vector<std::string> extensions;

    if (enableWindowing) {
        uint32_t extensionCount;
        const char **extensionNames = foeWindowGetVulkanExtensions(&extensionCount);
        for (int i = 0; i < extensionCount; ++i) {
            extensions.emplace_back(extensionNames[i]);
        }
    }

#ifdef FOE_XR_SUPPORT
    // OpenXR
    if (xrRuntime != FOE_NULL_HANDLE) {
        XrSystemId xrSystemId;
        std::vector<std::string> xrExtensions;

        XrResult xrRes =
            foeXrGetVulkanInstanceExtensions(foeXrOpenGetInstance(xrRuntime), xrExtensions);
        if (xrRes == XR_SUCCESS) {
            extensions.insert(extensions.end(), xrExtensions.begin(), xrExtensions.end());
        }

        // Add another that's missing??
        extensions.emplace_back("VK_KHR_external_fence_capabilities");
    }
#endif

    return foeGfxVkCreateRuntime("FoE Engine", 0, layers, extensions, validation, debugLogging,
                                 pGfxRuntime);
}

namespace {

auto determineVkPhysicalDevice(VkInstance vkInstance,
                               foeXrRuntime xrRuntime,
                               VkSurfaceKHR vkSurface,
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
        foeXrGetVulkanGraphicsDevice(foeXrOpenGetInstance(xrRuntime), 0, vkInstance,
                                     &xrPhysicalDevice);
    }
#endif

    // Window Requirements
    std::vector<VkBool32> supportsWindow(physicalDeviceCount, VK_FALSE);
    for (uint32_t i = 0; i < physicalDeviceCount; ++i) {
        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(physDevices[i], &queueFamilyCount, nullptr);

        for (uint32_t queueIndex = 0; queueIndex < queueFamilyCount; ++queueIndex) {
            vkGetPhysicalDeviceSurfaceSupportKHR(physDevices[i], queueIndex, vkSurface,
                                                 &supportsWindow[i]);

            if (supportsWindow[i] == VK_TRUE) {
                break;
            }
        }
    }

    // If we've been given an explicit GPU...
    if (explicitGpu != UINT32_MAX) {
        if (explicitGpu < physicalDeviceCount) {
            // It exists, spit out warning if it doesn't support OpenXR or windowing
            if (xrPhysicalDevice != VK_NULL_HANDLE &&
                xrPhysicalDevice != physDevices[explicitGpu]) {
                FOE_LOG(General, Warning,
                        "Explicit Physical GPU specified, OpenXR is possible but not supported on "
                        "this GPU: {}",
                        explicitGpu < physicalDeviceCount)
            }
            if (supportsWindow[explicitGpu] == VK_FALSE) {
                FOE_LOG(
                    General, Warning,
                    "Explicit Physical GPU specified, Windowing is not supported on this GPU: {}",
                    explicitGpu < physicalDeviceCount)
            }

            return physDevices[explicitGpu];
        } else {
            // Invalid device index given
            FOE_LOG(General, Error, "Explicit Physical GPU specified, but could not be found: {}",
                    explicitGpu);
        }
    }

    // First, try one that supports both OpenXR and Windowing
    for (uint32_t i = 0; i < physicalDeviceCount; ++i) {
        if (supportsWindow[i] == VK_TRUE && physDevices[i] == xrPhysicalDevice) {
            return xrPhysicalDevice;
        }
    }

    FOE_LOG(General, Verbose, "Failed to find device that supported both XR and windowing")

    // Second, try a XR-only device (if specified)
    if (forceXr) {
        if (xrPhysicalDevice != VK_NULL_HANDLE) {
            FOE_LOG(General, Info, "Using a VkPhysicalDevice that *only* supports XR");
            return xrPhysicalDevice;
        } else {
            FOE_LOG(General, Error,
                    "Attempted to use XR, however no physical device supports it currently");
        }
    }

    // Thirdly, try a Window-only capable device
    for (uint32_t i = 0; i < physicalDeviceCount; ++i) {
        if (supportsWindow[i] == VK_TRUE) {
            FOE_LOG(General, Info, "Using VkPhysicalDevice that only supports windowing")
            return physDevices[i];
        }
    }

    if (vkSurface == VK_NULL_HANDLE) {
        FOE_LOG(General, Info, "Using VkPhysicalDevice that doesn't support XR or Windowing");
        return physDevices[0];
    }

    FOE_LOG(General, Error, "Failed to find physical device that fit any requirements");
    return VK_NULL_HANDLE;
}

} // namespace

std::error_code createGfxSession(foeGfxRuntime gfxRuntime,
                                 foeXrRuntime xrRuntime,
                                 bool enableWindowing,
                                 std::vector<VkSurfaceKHR> windowSurfaces,
                                 uint32_t explicitGpu,
                                 bool forceXr,
                                 foeGfxSession *pGfxSession) {
    VkInstance vkInstance = foeGfxVkGetInstance(gfxRuntime);
    // Determine the physical device
    VkPhysicalDevice vkPhysicalDevice = determineVkPhysicalDevice(
        foeGfxVkGetInstance(gfxRuntime), xrRuntime, windowSurfaces[0], explicitGpu, forceXr);

    // Layers and Extensions
    std::vector<std::string> layers;
    std::vector<std::string> extensions;

    if (enableWindowing) {
        extensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

#ifdef FOE_XR_SUPPORT
    // OpenXR
    if (xrRuntime != FOE_NULL_HANDLE) {
        std::vector<std::string> xrExtensions;

        XrResult xrRes =
            foeXrGetVulkanDeviceExtensions(foeXrOpenGetInstance(xrRuntime), xrExtensions);
        if (xrRes == XR_SUCCESS) {
            extensions.insert(extensions.end(), xrExtensions.begin(), xrExtensions.end());
        }
    }
#endif

    return foeGfxVkCreateSession(gfxRuntime, vkPhysicalDevice, layers, extensions, pGfxSession);
}