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

struct foeGfxEnvironment {
    VkInstance instance;
    VkDebugReportCallbackEXT debugCallback;

    VkPhysicalDevice physicalDevice;
    VkDevice device;

    uint32_t numQueueFamilies;
    foeQueueFamily pQueueFamilies[MaxQueueFamilies];

    VmaAllocator allocator;
};

FOE_GFX_EXPORT VkResult foeGfxCreateEnvironment(bool validation,
                                                const char *appName,
                                                uint32_t appVersion,
                                                foeGfxEnvironment **ppEnvironment);

FOE_GFX_EXPORT void foeGfxDestroyEnvironment(foeGfxEnvironment *pEnvironment);

#endif // FOE_GRAPHICS_ENVIRONMENT_HPP