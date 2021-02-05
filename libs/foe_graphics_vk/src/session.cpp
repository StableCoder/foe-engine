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

#include <foe/graphics/vk/session.hpp>

#include <vk_error_code.hpp>

#include "log.hpp"
#include "runtime.hpp"
#include "session.hpp"

#include <memory>

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

void createQueueFamily(VkDevice device,
                       VkQueueFlags flags,
                       uint32_t family,
                       uint32_t numQueues,
                       foeGfxVkQueueFamily *pQueueFamily) {
    if (numQueues >= MaxQueuesPerFamily) {
        FOE_LOG(foeVkGraphics, Fatal,
                "There are {} Vulkan queue families, when the maximum compiled support is {}",
                numQueues, MaxQueuesPerFamily)
        std::abort();
    }

    pQueueFamily->flags = flags;
    pQueueFamily->family = family;
    pQueueFamily->numQueues = numQueues;

    for (auto &sync : pQueueFamily->sync) {
        sync.unlock();
    }

    for (uint32_t i = 0; i < numQueues; ++i) {
        vkGetDeviceQueue(device, family, i, &pQueueFamily->queue[i]);
    }
}

void foeGfxVkDestroySession(foeGfxVkSession *pSession) {
    // Descriptor Set Layouts
    pSession->builtinDescriptorSets.deinitialize(pSession->device);
    pSession->descriptorSetLayoutPool.deinitialize();

    if (pSession->allocator != VK_NULL_HANDLE)
        vmaDestroyAllocator(pSession->allocator);

    if (pSession->device != VK_NULL_HANDLE)
        vkDestroyDevice(pSession->device, nullptr);

    delete pSession;
}

} // namespace

std::error_code foeGfxVkCreateSession(foeGfxRuntime runtime,
                                      VkPhysicalDevice vkPhysicalDevice,
                                      std::vector<std::string> layers,
                                      std::vector<std::string> extensions,
                                      foeGfxSession *pSession) {
    auto *pNewSession = new foeGfxVkSession;
    pNewSession->instance = reinterpret_cast<foeGfxVkRuntime *>(runtime)->instance;
    pNewSession->physicalDevice = vkPhysicalDevice;

    // Queues
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &pNewSession->numQueueFamilies,
                                             nullptr);
    std::unique_ptr<VkQueueFamilyProperties[]> queueFamilyProperties(
        new VkQueueFamilyProperties[pNewSession->numQueueFamilies]);
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &pNewSession->numQueueFamilies,
                                             queueFamilyProperties.get());

    uint32_t maxQueueCount = 0;
    std::unique_ptr<VkDeviceQueueCreateInfo[]> queueCI(
        new VkDeviceQueueCreateInfo[pNewSession->numQueueFamilies]);
    for (uint32_t i = 0; i < pNewSession->numQueueFamilies; ++i) {
        queueCI[i] = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = i,
            .queueCount = queueFamilyProperties[i].queueCount,
        };
        maxQueueCount = std::max(maxQueueCount, queueFamilyProperties[i].queueCount);
    }

    std::unique_ptr<float[]> queuePriorities(new float[maxQueueCount]);
    std::fill_n(queuePriorities.get(), maxQueueCount, 0.f);

    for (uint32_t i = 0; i < pNewSession->numQueueFamilies; ++i) {
        queueCI[i].pQueuePriorities = queuePriorities.get();
    }

    std::vector<char const *> finalLayers;
    std::vector<char const *> finalExtensions;

    for (auto &it : layers)
        finalLayers.emplace_back(it.data());
    for (auto &it : extensions)
        finalExtensions.emplace_back(it.data());

    VkDeviceCreateInfo deviceCI{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = pNewSession->numQueueFamilies,
        .pQueueCreateInfos = queueCI.get(),
        .enabledLayerCount = static_cast<uint32_t>(finalLayers.size()),
        .ppEnabledLayerNames = finalLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(finalExtensions.size()),
        .ppEnabledExtensionNames = finalExtensions.data(),
    };

    VkResult vkRes = vkCreateDevice(vkPhysicalDevice, &deviceCI, nullptr, &pNewSession->device);

    // Retrieve the queues
    for (uint32_t i = 0; i < pNewSession->numQueueFamilies; ++i) {
        createQueueFamily(pNewSession->device, queueFamilyProperties[i].queueFlags, i,
                          queueFamilyProperties[i].queueCount, &pNewSession->pQueueFamilies[i]);
    }

    { // Allocator
        VmaAllocatorCreateInfo allocatorCI{
            .physicalDevice = pNewSession->physicalDevice,
            .device = pNewSession->device,
        };

        vkRes = vmaCreateAllocator(&allocatorCI, &pNewSession->allocator);
        if (vkRes != VK_SUCCESS) {
            goto CREATE_FAILED;
        }
    }

    { // Descriptor Set Layouts
        vkRes = pNewSession->descriptorSetLayoutPool.initialize(pNewSession->device);
        if (vkRes != VK_SUCCESS)
            goto CREATE_FAILED;

        vkRes = pNewSession->builtinDescriptorSets.initialize(
            pNewSession->device, &pNewSession->descriptorSetLayoutPool);
        if (vkRes != VK_SUCCESS)
            goto CREATE_FAILED;
    }

CREATE_FAILED:
    if (vkRes != VK_NULL_HANDLE) {
        foeGfxVkDestroySession(pNewSession);
    } else {
        *pSession = session_to_handle(pNewSession);
    }

    return vkRes;
}

uint32_t foeGfxVkGetBestQueue(foeGfxSession session, VkQueueFlags flags) {
    auto *pSession = session_from_handle(session);
    std::vector<std::pair<uint32_t, uint32_t>> compatibleQueueFamilies;

    for (uint32_t i = 0; i < MaxQueueFamilies; ++i) {
        if (pSession->pQueueFamilies[i].numQueues == 0)
            continue;

        if (pSession->pQueueFamilies[i].flags == flags) {
            return i;
        }
        if ((pSession->pQueueFamilies[i].flags & flags) == flags) {
            compatibleQueueFamilies.emplace_back(i, popcount(pSession->pQueueFamilies[i].flags));
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

VkInstance foeGfxVkGetInstance(foeGfxSession session) {
    auto *pSession = session_from_handle(session);
    return pSession->instance;
}

VkPhysicalDevice foeGfxVkGetPhysicalDevice(foeGfxSession session) {
    auto *pSession = session_from_handle(session);
    return pSession->physicalDevice;
}

VkDevice foeGfxVkGetDevice(foeGfxSession session) {
    auto *pSession = session_from_handle(session);
    return pSession->device;
}

VmaAllocator foeGfxVkGetAllocator(foeGfxSession session) {
    auto *pSession = session_from_handle(session);
    return pSession->allocator;
}

foeGfxVkQueueFamily *getFirstQueue(foeGfxSession session) {
    auto *pSession = session_from_handle(session);
    return &pSession->pQueueFamilies[0];
}

void foeGfxDestroySession(foeGfxSession session) {
    auto *pSession = session_from_handle(session);
    foeGfxVkDestroySession(pSession);
}

auto foeGfxVkGetDescriptorSetLayout(foeGfxSession session,
                                    VkDescriptorSetLayoutCreateInfo const *pDescriptorSetLayoutCI)
    -> VkDescriptorSetLayout {
    auto *pSession = session_from_handle(session);

    return pSession->descriptorSetLayoutPool.get(pDescriptorSetLayoutCI);
}

auto foeGfxVkGetBuiltinLayout(foeGfxSession session,
                              foeBuiltinDescriptorSetLayoutFlags builtinLayout)
    -> VkDescriptorSetLayout {
    auto *pSession = session_from_handle(session);

    return pSession->builtinDescriptorSets.getBuiltinLayout(builtinLayout);
}

auto foeGfxVkGetBuiltinSetLayoutIndex(foeGfxSession session,
                                      foeBuiltinDescriptorSetLayoutFlags builtinLayout)
    -> uint32_t {
    auto *pSession = session_from_handle(session);

    return pSession->builtinDescriptorSets.getBuiltinSetLayoutIndex(builtinLayout);
}