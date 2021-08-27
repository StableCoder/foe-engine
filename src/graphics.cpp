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
#include <foe/wsi/vulkan.hpp>
#include <vk_error_code.hpp>

#include "error_code.hpp"
#include "log.hpp"

#include <memory>

#ifdef FOE_XR_SUPPORT
#include <foe/xr/error_code.hpp>
#include <foe/xr/openxr/runtime.hpp>
#include <foe/xr/vulkan.hpp>
#endif

std::error_code createGfxRuntime(foeXrRuntime xrRuntime,
                                 bool enableWindowing,
                                 bool validation,
                                 bool debugLogging,
                                 foeGfxRuntime *pGfxRuntime) {
    std::error_code errC;
    std::vector<std::string> layers;
    std::vector<std::string> extensions;

    if (enableWindowing) {
        uint32_t extensionCount;
        const char **extensionNames;
        errC = foeWsiWindowGetVulkanExtensions(&extensionCount, &extensionNames);
        if (errC)
            return errC;

        for (uint32_t i = 0; i < extensionCount; ++i) {
            extensions.emplace_back(extensionNames[i]);
        }
    }

#ifdef FOE_XR_SUPPORT
    // OpenXR
    if (xrRuntime != FOE_NULL_HANDLE) {
        std::vector<std::string> xrExtensions;

        XrResult xrRes =
            foeXrGetVulkanInstanceExtensions(foeXrOpenGetInstance(xrRuntime), xrExtensions);
        if (xrRes != XR_SUCCESS)
            return xrRes;

        extensions.insert(extensions.end(), xrExtensions.begin(), xrExtensions.end());

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
        foeXrGetVulkanGraphicsDevice(foeXrOpenGetInstance(xrRuntime), 0, vkInstance,
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
                FOE_LOG(General, Warning,
                        "Explicit Physical GPU specified, OpenXR is possible but not supported on "
                        "this GPU: {}",
                        explicitGpu < physicalDeviceCount)
            }
            if (supportsWindow[explicitGpu] == VK_FALSE) {
                FOE_LOG(General, Warning,
                        "Explicit Physical GPU specified, Windowing is not supported on this "
                        "GPU: {}",
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

    if (surfaceCount == 0) {
        FOE_LOG(General, Info,
                "Using VkPhysicalDevice that doesn't necessarily support XR or Windowing");
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
    // Determine the physical device
    VkPhysicalDevice vkPhysicalDevice =
        determineVkPhysicalDevice(foeGfxVkGetInstance(gfxRuntime), xrRuntime, windowSurfaces.size(),
                                  windowSurfaces.data(), explicitGpu, forceXr);
    if (vkPhysicalDevice == VK_NULL_HANDLE)
        return FOE_BRINGUP_ERROR_NO_PHYSICAL_DEVICE_MEETS_REQUIREMENTS;

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