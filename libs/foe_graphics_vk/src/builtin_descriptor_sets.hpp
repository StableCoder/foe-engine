// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BUILTIN_DESCRIPTOR_SETS_HPP
#define BUILTIN_DESCRIPTOR_SETS_HPP

#include <foe/graphics/builtin_descriptor_sets.h>
#include <foe/graphics/export.h>
#include <vulkan/vulkan.h>

#include <array>

class foeGfxVkDescriptorSetLayoutPool;

/** Special builtin DescriptorSetLayouts and 'dummy' set
 *
 * These are core items that are integral to how most of the rendering and shared across many
 * generated shader programs.
 */
class foeGfxVkBuiltinDescriptorSets {
  public:
    auto initialize(VkDevice device, foeGfxVkDescriptorSetLayoutPool *pDescriptorSetLayoutPool)
        -> VkResult;
    void deinitialize(VkDevice device);

    auto getBuiltinLayout(foeBuiltinDescriptorSetLayoutFlags builtinLayout) const noexcept
        -> VkDescriptorSetLayout;

    auto getBuiltinSetLayoutIndex(foeBuiltinDescriptorSetLayoutFlags builtinLayout) const noexcept
        -> uint32_t;

    FOE_GFX_EXPORT
    auto getDummyLayout() const noexcept -> VkDescriptorSetLayout;
    FOE_GFX_EXPORT
    auto getDummySet() const noexcept -> VkDescriptorSet;

  private:
    std::array<VkDescriptorSetLayout, 3> mBuiltinLayouts;

    VkDescriptorPool mDescriptorPool{VK_NULL_HANDLE};

    VkDescriptorSetLayout mDummyLayout{VK_NULL_HANDLE};
    VkDescriptorSet mDummySet{VK_NULL_HANDLE};
};

#endif // BUILTIN_DESCRIPTOR_SETS_HPP