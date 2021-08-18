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
#include <foe/graphics/type_defs.hpp>
#include <foe/graphics/vk/fragment_descriptor_pool.hpp>
#include <foe/graphics/vk/pipeline_pool.hpp>
#include <foe/graphics/vk/queue_family.hpp>
#include <foe/graphics/vk/render_pass_pool.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <mutex>

#include "builtin_descriptor_sets.hpp"
#include "descriptor_set_layout_pool.hpp"

struct foeGfxVkSession {
    // From the Runtime
    VkInstance instance{VK_NULL_HANDLE};

    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
    VkDevice device{VK_NULL_HANDLE};
    VmaAllocator allocator{VK_NULL_HANDLE};

    uint32_t numQueueFamilies{0};
    foeGfxVkQueueFamily pQueueFamilies[MaxQueueFamilies];

    foeGfxVkBuiltinDescriptorSets builtinDescriptorSets;
    foeGfxVkDescriptorSetLayoutPool descriptorSetLayoutPool;

    foeGfxVkRenderPassPool renderPassPool;
    foeGfxVkFragmentDescriptorPool fragmentDescriptorPool;
    foeGfxVkPipelinePool pipelinePool;
};

FOE_DEFINE_HANDLE_CASTS(session, foeGfxVkSession, foeGfxSession)

#endif // SESSION_HPP