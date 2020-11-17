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

#include <foe/graphics/environment.hpp>
#include <foe/wsi_vulkan.hpp>

#include <bit>
#include <cassert>
#include <limits>
#include <memory>
#include <tuple>
#include <vector>

#include "gfx_log.hpp"

namespace {

/// @todo Replace all with std::popcount when integrated across all platforms
inline uint32_t popcount(uint32_t value) noexcept {
#ifdef __GNUC__
    return __builtin_popcountll(value);
#elif __APPLE__
    return __popcount(value);
#else // WIN32
    uint32_t ret = 0;
    for (int i = 0; i < sizeof(value) * 8; ++i) {
        if (value & 0x1) {
            ++ret;
        }
        value >>= 1;
    }

    return ret;
#endif
}

VkBool32 vulkanMessageCallbacks(VkDebugReportFlagsEXT flags,
                                VkDebugReportObjectTypeEXT objectType,
                                uint64_t object,
                                size_t location,
                                int32_t messageCode,
                                const char *pLayerPrefix,
                                const char *pMessage,
                                void *pUserData) {
    if ((flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) != 0) {
        FOE_LOG(Graphics, Error, "[{}] Code {} : {}", pLayerPrefix, messageCode, pMessage)
    }
    if ((flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) != 0 ||
        (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) != 0) {
        FOE_LOG(Graphics, Warning, "[{}] Code {} : {}", pLayerPrefix, messageCode, pMessage)
    }
    if ((flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) != 0) {
        FOE_LOG(Graphics, Info, "[{}] Code {} : {}", pLayerPrefix, messageCode, pMessage)
    }
    if ((flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) != 0) {
        FOE_LOG(Graphics, Verbose, "[{}] Code {} : {}", pLayerPrefix, messageCode, pMessage)
    }

    return VK_FALSE;
}

void clearEnvironment(foeGfxEnvironment *pEnvironment) {
    pEnvironment->instance = VK_NULL_HANDLE;
    pEnvironment->debugCallback = VK_NULL_HANDLE;

    pEnvironment->physicalDevice = VK_NULL_HANDLE;
    pEnvironment->device = VK_NULL_HANDLE;

    pEnvironment->numQueueFamilies = 0;
    for (auto &it : pEnvironment->pQueueFamilies) {
        it.flags = 0;
        it.family = 0;
        it.numQueues = 0;
    }
}

void createQueueFamily(VkDevice device,
                       VkQueueFlags flags,
                       uint32_t family,
                       uint32_t numQueues,
                       foeQueueFamily *pQueueFamily) {
    assert(numQueues < MaxQueuesPerFamily);

    std::fill_n(pQueueFamily->queue, MaxQueuesPerFamily, static_cast<VkQueue>(VK_NULL_HANDLE));

    pQueueFamily->flags = flags;
    pQueueFamily->family = family;
    pQueueFamily->numQueues = numQueues;

    for (uint32_t i = 0; i < numQueues; ++i) {
        pQueueFamily->sync[i].unlock();
        vkGetDeviceQueue(device, family, i, &pQueueFamily->queue[i]);
    }
}

} // namespace

VkResult foeGfxCreateEnvironment(bool validation,
                                 const char *appName,
                                 uint32_t appVersion,
                                 foeGfxEnvironment **ppEnvironment) {
    VkResult res;
    foeGfxEnvironment *pEnv = new foeGfxEnvironment;
    clearEnvironment(pEnv);

    /// Instance
    VkApplicationInfo appinfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = appName,
        .applicationVersion = appVersion,
        .pEngineName = "FoE-Engine",
        .engineVersion = 0,
        .apiVersion = VK_MAKE_VERSION(1, 0, 0),
    };

    std::vector<const char *> extensions;
    std::vector<const char *> layers;

    uint32_t extensionCount;
    const char **extensionNames = foeWindowGetVulkanExtensions(&extensionCount);

    for (int i = 0; i < extensionCount; ++i) {
        extensions.emplace_back(extensionNames[i]);
    }

    if (validation) {
        extensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    if (validation) {
        layers.emplace_back("VK_LAYER_KHRONOS_validation");
    }

    VkInstanceCreateInfo instanceCI{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appinfo,
        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

    res = vkCreateInstance(&instanceCI, nullptr, &pEnv->instance);
    if (res != VK_SUCCESS)
        return res;

    if (validation) {
        VkDebugReportCallbackCreateInfoEXT debugReportCI{
            .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
            .flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                     VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
                     VK_DEBUG_REPORT_INFORMATION_BIT_EXT,
            .pfnCallback = &vulkanMessageCallbacks,
        };

        auto fpCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
            vkGetInstanceProcAddr(pEnv->instance, "vkCreateDebugReportCallbackEXT"));

        res = fpCreateDebugReportCallbackEXT(pEnv->instance, &debugReportCI, nullptr,
                                             &pEnv->debugCallback);
        if (res != VK_SUCCESS) {
            foeGfxDestroyEnvironment(pEnv);
            return res;
        }
    }

    // Physical Device
    uint32_t physicalDeviceCount;
    res = vkEnumeratePhysicalDevices(pEnv->instance, &physicalDeviceCount, nullptr);
    if (res != VK_SUCCESS) {
        foeGfxDestroyEnvironment(pEnv);
        return res;
    }

    std::unique_ptr<VkPhysicalDevice[]> physDevices(new VkPhysicalDevice[physicalDeviceCount]);
    res = vkEnumeratePhysicalDevices(pEnv->instance, &physicalDeviceCount, physDevices.get());
    if (res != VK_SUCCESS) {
        foeGfxDestroyEnvironment(pEnv);
        return res;
    }
    pEnv->physicalDevice = physDevices[0];

    // Queues
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(pEnv->physicalDevice, &queueFamilyCount, nullptr);
    std::unique_ptr<VkQueueFamilyProperties[]> queueFamilyProperties(
        new VkQueueFamilyProperties[queueFamilyCount]);
    vkGetPhysicalDeviceQueueFamilyProperties(pEnv->physicalDevice, &queueFamilyCount,
                                             queueFamilyProperties.get());

    uint32_t maxQueueCount = 0;
    std::unique_ptr<VkDeviceQueueCreateInfo[]> queueCI(
        new VkDeviceQueueCreateInfo[queueFamilyCount]);
    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        queueCI[i] = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = i,
            .queueCount = queueFamilyProperties[i].queueCount,
        };
        maxQueueCount = std::max(maxQueueCount, queueFamilyProperties[i].queueCount);
    }

    std::unique_ptr<float> queuePriorities(new float[maxQueueCount]);
    std::fill_n(queuePriorities.get(), maxQueueCount, 0.f);

    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        queueCI[i].pQueuePriorities = queuePriorities.get();
    }

    // Device
    const char *deviceExtensions = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

    VkDeviceCreateInfo deviceCI{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = queueFamilyCount,
        .pQueueCreateInfos = queueCI.get(),
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = &deviceExtensions,
    };

    res = vkCreateDevice(physDevices[0], &deviceCI, nullptr, &pEnv->device);
    if (res != VK_SUCCESS) {
        foeGfxDestroyEnvironment(pEnv);
        return res;
    }

    pEnv->numQueueFamilies = queueFamilyCount;
    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        createQueueFamily(pEnv->device, queueFamilyProperties[i].queueFlags, i,
                          queueFamilyProperties[i].queueCount, &pEnv->pQueueFamilies[i]);
    }

    // VMA Allocator
    VmaAllocatorCreateInfo allocatorCI{
        .physicalDevice = pEnv->physicalDevice,
        .device = pEnv->device,
    };

    res = vmaCreateAllocator(&allocatorCI, &pEnv->allocator);
    if (res != VK_SUCCESS) {
        foeGfxDestroyEnvironment(pEnv);
        return res;
    }

    *ppEnvironment = pEnv;

    return res;
}

void foeGfxDestroyEnvironment(foeGfxEnvironment *pEnvironment) {
    if (pEnvironment->allocator != VK_NULL_HANDLE) {
        vmaDestroyAllocator(pEnvironment->allocator);
    }

    if (pEnvironment->device != VK_NULL_HANDLE) {
        vkDestroyDevice(pEnvironment->device, nullptr);
    }

    if (pEnvironment->debugCallback != VK_NULL_HANDLE) {
        auto fpDestroyDebugReportCallbackEXT =
            reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
                vkGetInstanceProcAddr(pEnvironment->instance, "vkDestroyDebugReportCallbackEXT"));

        fpDestroyDebugReportCallbackEXT(pEnvironment->instance, pEnvironment->debugCallback,
                                        nullptr);
    }

    if (pEnvironment->instance != VK_NULL_HANDLE) {
        vkDestroyInstance(pEnvironment->instance, nullptr);
    }
}

uint32_t foeGfxGetBestQueue(foeGfxEnvironment const *pEnvironment, VkQueueFlags flags) {
    std::vector<std::pair<uint32_t, uint32_t>> compatibleQueueFamilies;

    for (uint32_t i = 0; i < MaxQueueFamilies; ++i) {
        if (pEnvironment->pQueueFamilies[i].flags == flags) {
            return i;
        }
        if ((pEnvironment->pQueueFamilies[i].flags & flags) == flags) {
            compatibleQueueFamilies.emplace_back(i,
                                                 popcount(pEnvironment->pQueueFamilies[i].flags));
        }
    }

    // Now iterate through the list, find the queue with the least number of extra flags
    if (compatibleQueueFamilies.empty()) {
        return std::numeric_limits<uint32_t>::max();
    }

    uint32_t leastFlagsIndex = 0;
    uint32_t leastFlags = compatibleQueueFamilies[0].second;

    for (uint32_t i = 1; i < compatibleQueueFamilies.size(); ++i) {
        if (compatibleQueueFamilies[i].second < leastFlags) {
            leastFlagsIndex = i;
            leastFlags = compatibleQueueFamilies[i].second;
        }
    }

    return compatibleQueueFamilies[leastFlagsIndex].first;
}