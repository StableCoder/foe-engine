// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef SESSION_HPP
#define SESSION_HPP

#include <foe/graphics/session.h>
#include <foe/graphics/type_defs.h>
#include <foe/graphics/vk/fragment_descriptor_pool.hpp>
#include <foe/graphics/vk/pipeline_pool.h>
#include <foe/graphics/vk/render_pass_pool.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "builtin_descriptor_sets.hpp"
#include "descriptor_set_layout_pool.hpp"
#include "queue_family.hpp"

#include <mutex>

struct foeGfxVkSession {
    // From the Runtime
    VkInstance instance{VK_NULL_HANDLE};

    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
    VkDevice device{VK_NULL_HANDLE};
    VmaAllocator allocator{VK_NULL_HANDLE};

    uint32_t numQueueFamilies{0};
    QueueFamily queueFamilies[MaxQueueFamilies];

    /// Length in bytes of pLayerNames
    uint32_t layerNamesLength{0};
    /// Set of strings representing the instance's layers, delimited by NULL characters
    char *pLayerNames{nullptr};
    /// Length in bytes of pExtensionNames
    uint32_t extensionNamesLength{0};
    /// Set of strings representing the instance's extensions, delimited by NULL characters
    char *pExtensionNames{nullptr};

    /// Set of enabled Vulkan 1.0 features
    VkPhysicalDeviceFeatures features_1_0;
#ifdef VK_VERSION_1_1
    /// Set of enabled Vulkan 1.1 features
    VkPhysicalDeviceVulkan11Features features_1_1;
#endif
#ifdef VK_VERSION_1_2
    /// Set of enabled Vulkan 1.2 features
    VkPhysicalDeviceVulkan12Features features_1_2;
#endif
#ifdef VK_VERSION_1_3
    // Set of enabled Vulkan 1.3 features
    VkPhysicalDeviceVulkan13Features features_1_3;
#endif

    foeGfxVkBuiltinDescriptorSets builtinDescriptorSets;
    foeGfxVkDescriptorSetLayoutPool descriptorSetLayoutPool;

    foeGfxVkRenderPassPool renderPassPool;
    foeGfxVkFragmentDescriptorPool fragmentDescriptorPool;
    foeGfxVkPipelinePool pipelinePool{FOE_NULL_HANDLE};
};

FOE_DEFINE_HANDLE_CASTS(session, foeGfxVkSession, foeGfxSession)

#endif // SESSION_HPP