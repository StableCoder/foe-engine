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

#ifndef FOE_GRAPHICS_ENVIRONMENT_HPP
#define FOE_GRAPHICS_ENVIRONMENT_HPP

#include <foe/graphics/export.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <mutex>
#include <string>
#include <vector>

enum {
    MaxQueueFamilies = 8U,
    MaxQueuesPerFamily = 8U,
};

struct foeQueueFamily {
    VkQueueFlags flags;
    uint32_t family;
    uint32_t numQueues;

    std::mutex sync[MaxQueuesPerFamily];
    VkQueue queue[MaxQueuesPerFamily];
};

struct foeVkDeviceEnvironment {
    VkInstance instance;

    VkPhysicalDevice physicalDevice;
    VkDevice device;

    VkPhysicalDeviceLimits physicalDeviceLimits;

    uint32_t numQueueFamilies;
    foeQueueFamily pQueueFamilies[MaxQueueFamilies];

    VmaAllocator allocator;
};

FOE_GFX_EXPORT VkResult foeVkCreateInstance(char const *appName,
                                            uint32_t appVersion,
                                            std::vector<std::string> layers,
                                            std::vector<std::string> extensions,
                                            VkInstance *pInstance);

FOE_GFX_EXPORT VkResult foeGfxCreateEnvironment(VkInstance vkInstance,
                                                VkPhysicalDevice vkPhysicalDevice,
                                                std::vector<std::string> deviceLayers,
                                                std::vector<std::string> deviceExtensions,
                                                foeVkDeviceEnvironment **ppEnvironment);

FOE_GFX_EXPORT void foeGfxDestroyEnvironment(foeVkDeviceEnvironment *pEnvironment);

FOE_GFX_EXPORT uint32_t foeGfxGetBestQueue(foeVkDeviceEnvironment const *pEnvironment,
                                           VkQueueFlags flags);

#endif // FOE_GRAPHICS_ENVIRONMENT_HPP