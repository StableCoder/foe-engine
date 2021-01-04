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

#include <foe/wsi_vulkan.hpp>

#include <memory>

auto determineVkInstanceEnvironment(EngineSettings const &engineSettings)
    -> std::tuple<std::vector<std::string>, std::vector<std::string>> {
    std::vector<std::string> layers;
    std::vector<std::string> extensions;

    // Windowing
    if (!engineSettings.window.haveWindow) {
        uint32_t extensionCount;
        const char **extensionNames = foeWindowGetVulkanExtensions(&extensionCount);
        for (int i = 0; i < extensionCount; ++i) {
            extensions.emplace_back(extensionNames[i]);
        }
    }

    // Validation
    if (engineSettings.graphics.validation) {
        layers.emplace_back("VK_LAYER_KHRONOS_validation");
    }

    // Debug Callback
    if (engineSettings.graphics.debugLogging) {
        extensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    return std::make_tuple(layers, extensions);
}

auto determineVkPhysicalDevice(EngineSettings const &engineSettings, VkInstance vkInstance)
    -> VkPhysicalDevice {
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

    if (engineSettings.graphics.gpu < physicalDeviceCount) {
        return physDevices[engineSettings.graphics.gpu];
    }

    return physDevices[0];
}

auto determineVkDeviceEnvironment(EngineSettings const &engineSettings)
    -> std::tuple<std::vector<std::string>, std::vector<std::string>> {
    std::vector<std::string> layers;
    std::vector<std::string> extensions;

    // Swapchain (for windowing)
    if (!engineSettings.window.haveWindow) {
        extensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    return std::make_tuple(layers, extensions);
}