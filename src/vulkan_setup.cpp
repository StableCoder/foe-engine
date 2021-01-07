/*
    Copyright (C) 2020 George Cave.

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

#include "vulkan_setup.hpp"

#include <foe/log.hpp>
#include <foe/wsi_vulkan.hpp>
#include <foe/xr/vulkan.hpp>

#include <memory>

auto determineVkInstanceEnvironment(XrInstance xrInstance,
                                    XrSystemId xrSystemId,
                                    bool enableWindowing,
                                    bool validation,
                                    bool debugLogging)
    -> std::tuple<std::vector<std::string>, std::vector<std::string>> {
    std::vector<std::string> layers;
    std::vector<std::string> extensions;

    // Windowing
    if (enableWindowing) {
        uint32_t extensionCount;
        const char **extensionNames = foeWindowGetVulkanExtensions(&extensionCount);
        for (int i = 0; i < extensionCount; ++i) {
            extensions.emplace_back(extensionNames[i]);
        }
    }

    // OpenXR
    if (xrInstance != XR_NULL_HANDLE) {
        std::vector<std::string> xrExtensions;
        foeXrGetVulkanInstanceExtensions(xrInstance, xrSystemId, xrExtensions);

        extensions.insert(extensions.end(), xrExtensions.begin(), xrExtensions.end());

        // Add another that's missing??
        extensions.emplace_back("VK_KHR_external_fence_capabilities");
    }

    // Validation
    if (validation) {
        layers.emplace_back("VK_LAYER_KHRONOS_validation");
    }

    // Debug Callback
    if (debugLogging) {
        extensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    return std::make_tuple(layers, extensions);
}

auto determineVkPhysicalDevice(VkInstance vkInstance,
                               XrInstance xrInstance,
                               XrSystemId xrSystemId,
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
    if (xrInstance != XR_NULL_HANDLE) {
        foeXrGetVulkanGraphicsDevice(xrInstance, xrSystemId, vkInstance, &xrPhysicalDevice);
    }

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
            if (xrPhysicalDevice != XR_NULL_HANDLE &&
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

auto determineVkDeviceEnvironment(XrInstance xrInstance,
                                  XrSystemId xrSystemId,
                                  bool enableWindowing)
    -> std::tuple<std::vector<std::string>, std::vector<std::string>> {
    std::vector<std::string> layers;
    std::vector<std::string> extensions;

    // Swapchain (for windowing)
    if (enableWindowing) {
        extensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    // OpenXR
    if (xrInstance != XR_NULL_HANDLE) {
        std::vector<std::string> xrExtensions;
        foeXrGetVulkanDeviceExtensions(xrInstance, xrSystemId, xrExtensions);

        extensions.insert(extensions.end(), xrExtensions.begin(), xrExtensions.end());
    }

    return std::make_tuple(layers, extensions);
}