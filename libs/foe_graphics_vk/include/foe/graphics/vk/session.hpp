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

#ifndef FOE_GRAPHICS_VK_SESSION_HPP
#define FOE_GRAPHICS_VK_SESSION_HPP

#include <foe/graphics/builtin_descriptor_sets.hpp>
#include <foe/graphics/export.h>
#include <foe/graphics/runtime.hpp>
#include <foe/graphics/session.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <string>
#include <system_error>
#include <vector>

class foeGfxVkRenderPassPool;
class foeGfxVkFragmentDescriptorPool;

FOE_GFX_EXPORT std::error_code foeGfxVkCreateSession(foeGfxRuntime runtime,
                                                     VkPhysicalDevice vkPhysicalDevice,
                                                     std::vector<std::string> layers,
                                                     std::vector<std::string> extensions,
                                                     foeGfxSession *pSession);

FOE_GFX_EXPORT VkInstance foeGfxVkGetInstance(foeGfxSession session);
FOE_GFX_EXPORT VkPhysicalDevice foeGfxVkGetPhysicalDevice(foeGfxSession session);
FOE_GFX_EXPORT VkDevice foeGfxVkGetDevice(foeGfxSession session);
FOE_GFX_EXPORT VmaAllocator foeGfxVkGetAllocator(foeGfxSession session);

FOE_GFX_EXPORT uint32_t foeGfxVkGetBestQueue(foeGfxSession session, VkQueueFlags flags);

#include <foe/graphics/vk/queue_family.hpp>
FOE_GFX_EXPORT foeGfxVkQueueFamily *getFirstQueue(foeGfxSession session);

FOE_GFX_EXPORT auto foeGfxVkGetDummySet(foeGfxSession session) -> VkDescriptorSet;

FOE_GFX_EXPORT auto foeGfxVkGetBuiltinLayout(foeGfxSession session,
                                             foeBuiltinDescriptorSetLayoutFlags builtinLayout)
    -> VkDescriptorSetLayout;

FOE_GFX_EXPORT auto foeGfxVkGetBuiltinSetLayoutIndex(
    foeGfxSession session, foeBuiltinDescriptorSetLayoutFlags builtinLayout) -> uint32_t;

FOE_GFX_EXPORT auto foeGfxVkGetRenderPassPool(foeGfxSession session) -> foeGfxVkRenderPassPool *;

FOE_GFX_EXPORT auto foeGfxVkGetFragmentDescriptorPool(foeGfxSession session)
    -> foeGfxVkFragmentDescriptorPool *;

FOE_GFX_EXPORT auto foeGfxVkGetSupportedMSAA(foeGfxSession session) -> VkSampleCountFlags;
FOE_GFX_EXPORT auto foeGfxVkGetMaxSupportedMSAA(foeGfxSession session) -> VkSampleCountFlags;

#endif // FOE_GRAPHICS_VK_SESSION_HPP