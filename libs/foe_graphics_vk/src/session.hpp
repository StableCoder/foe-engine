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

#ifndef SESSION_HPP
#define SESSION_HPP

#include <foe/graphics/session.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <mutex>

#include <foe/graphics/type_defs.hpp>

struct foeGfxVkSession {
    // From the Runtime
    VkInstance instance{VK_NULL_HANDLE};

    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
    VkDevice device{VK_NULL_HANDLE};
    VmaAllocator allocator{VK_NULL_HANDLE};

    uint32_t numQueueFamilies{0};
    foeVkQueueFamily pQueueFamilies[MaxQueueFamilies];
};

FOE_DEFINE_HANDLE_CASTS(session, foeGfxVkSession, foeGfxSession)

#endif // SESSION_HPP