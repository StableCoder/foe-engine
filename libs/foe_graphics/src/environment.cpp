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
#include <foe/wsi.hpp>

#include <cassert>
#include <memory>

namespace {

void clearEnvironment(foeGfxEnvironment *pEnvironment) {
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
        pQueueFamily->sync[i].unlock();
        vkGetDeviceQueue(device, family, i, &pQueueFamily->queue[i]);
    }
}

} // namespace

VkResult foeGfxCreateEnvironment(const char *appName,
                                 uint32_t appVersion,
                                 foeGfxEnvironment **ppEnvironment) {
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

    // GLFW Instance Extensions
    uint32_t extensionCount;
    const char **extensionNames = foeWindowGetVulkanExtensions(&extensionCount);

    VkInstanceCreateInfo instanceCI{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appinfo,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = extensionCount,
        .ppEnabledExtensionNames = extensionNames,
    };

    VkResult res = vkCreateInstance(&instanceCI, nullptr, &pEnv->instance);

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
    VkDeviceCreateInfo deviceCI{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = queueFamilyCount,
        .pQueueCreateInfos = queueCI.get(),
    };

    res = vkCreateDevice(physDevices[0], &deviceCI, nullptr, &pEnv->device);
    if (res != VK_SUCCESS) {
        foeGfxDestroyEnvironment(pEnv);
        return res;
    }

    uint32_t numQueueFamilies = queueFamilyCount;
    for (int i = 0; i < numQueueFamilies; ++i) {
        createQueueFamily(pEnv->device, queueFamilyProperties[i].queueFlags, i,
                          queueFamilyProperties[i].queueCount, &pEnv->pQueueFamilies[i]);
    }

    *ppEnvironment = pEnv;

    return res;
}

void foeGfxDestroyEnvironment(foeGfxEnvironment *pEnvironment) {
    vkDestroyDevice(pEnvironment->device, nullptr);
    vkDestroyInstance(pEnvironment->instance, nullptr);
}