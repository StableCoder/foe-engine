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

#include <foe/graphics/device_environment.hpp>

#include <foe/engine_detail.h>

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

void clearEnvironment(foeVkDeviceEnvironment *pEnvironment) {
    pEnvironment->instance = VK_NULL_HANDLE;

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
        vkGetDeviceQueue(device, family, i, &pQueueFamily->queue[i]);
    }
}

VkResult foeVkCreateDevice(VkPhysicalDevice physicalDevice,
                           std::vector<std::string> layers,
                           std::vector<std::string> extensions,
                           VkDevice *pDevice) {
    // Queues
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::unique_ptr<VkQueueFamilyProperties[]> queueFamilyProperties(
        new VkQueueFamilyProperties[queueFamilyCount]);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
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

    std::unique_ptr<float[]> queuePriorities(new float[maxQueueCount]);
    std::fill_n(queuePriorities.get(), maxQueueCount, 0.f);

    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        queueCI[i].pQueuePriorities = queuePriorities.get();
    }

    std::vector<char const *> finalLayers;
    std::vector<char const *> finalExtensions;

    for (auto &it : layers) {
        finalLayers.emplace_back(it.data());
    }
    for (auto &it : extensions) {
        finalExtensions.emplace_back(it.data());
    }

    VkDeviceCreateInfo deviceCI{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = queueFamilyCount,
        .pQueueCreateInfos = queueCI.get(),
        .enabledLayerCount = static_cast<uint32_t>(finalLayers.size()),
        .ppEnabledLayerNames = finalLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(finalExtensions.size()),
        .ppEnabledExtensionNames = finalExtensions.data(),
    };

    return vkCreateDevice(physicalDevice, &deviceCI, nullptr, pDevice);
}

} // namespace

VkResult foeVkCreateInstance(char const *appName,
                             uint32_t appVersion,
                             std::vector<std::string> layers,
                             std::vector<std::string> extensions,
                             VkInstance *pInstance) {
    VkApplicationInfo appinfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = appName,
        .applicationVersion = appVersion,
        .pEngineName = FOE_ENGINE_NAME,
        .engineVersion = FOE_ENGINE_VERSION,
        .apiVersion = VK_MAKE_VERSION(1, 0, 0),
    };

    std::vector<const char *> finalLayers;
    std::vector<const char *> finalExtensions;

    for (auto &it : layers) {
        finalLayers.emplace_back(it.data());
    }
    for (auto &it : extensions) {
        finalExtensions.emplace_back(it.data());
    }

    VkInstanceCreateInfo instanceCI{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appinfo,
        .enabledLayerCount = static_cast<uint32_t>(finalLayers.size()),
        .ppEnabledLayerNames = finalLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(finalExtensions.size()),
        .ppEnabledExtensionNames = finalExtensions.data(),
    };

    return vkCreateInstance(&instanceCI, nullptr, pInstance);
}

VkResult foeGfxCreateEnvironment(VkInstance vkInstance,
                                 VkPhysicalDevice vkPhysicalDevice,
                                 std::vector<std::string> deviceLayers,
                                 std::vector<std::string> deviceExtensions,
                                 foeVkDeviceEnvironment **ppEnvironment) {
    VkResult res;
    foeVkDeviceEnvironment *pEnv = new foeVkDeviceEnvironment;
    clearEnvironment(pEnv);

    /// Instance / Physical Device
    pEnv->instance = vkInstance;
    pEnv->physicalDevice = vkPhysicalDevice;

    VkPhysicalDeviceProperties devProperties;
    vkGetPhysicalDeviceProperties(pEnv->physicalDevice, &devProperties);
    pEnv->physicalDeviceLimits = devProperties.limits;

    // Device
    res = foeVkCreateDevice(vkPhysicalDevice, deviceLayers, deviceExtensions, &pEnv->device);
    if (res != VK_SUCCESS) {
        foeGfxDestroyEnvironment(pEnv);
        return res;
    }

    // Queues
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, nullptr);
    std::unique_ptr<VkQueueFamilyProperties[]> queueFamilyProperties(
        new VkQueueFamilyProperties[queueFamilyCount]);
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount,
                                             queueFamilyProperties.get());

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

void foeGfxDestroyEnvironment(foeVkDeviceEnvironment *pEnvironment) {
    if (pEnvironment->allocator != VK_NULL_HANDLE) {
        vmaDestroyAllocator(pEnvironment->allocator);
    }

    if (pEnvironment->device != VK_NULL_HANDLE) {
        vkDestroyDevice(pEnvironment->device, nullptr);
    }
}

uint32_t foeGfxGetBestQueue(foeVkDeviceEnvironment const *pEnvironment, VkQueueFlags flags) {
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