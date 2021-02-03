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

#ifndef BUILTIN_DESCRIPTOR_SETS_HPP
#define BUILTIN_DESCRIPTOR_SETS_HPP

#include <foe/graphics/builtin_descriptor_sets.hpp>
#include <foe/graphics/export.h>
#include <vulkan/vulkan.h>

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

    auto getDummyLayout() const noexcept -> VkDescriptorSetLayout;
    auto getDummySet() const noexcept -> VkDescriptorSet;

  private:
    std::array<VkDescriptorSetLayout, 3> mBuiltinLayouts;

    VkDescriptorPool mDescriptorPool{VK_NULL_HANDLE};

    VkDescriptorSetLayout mDummyLayout{VK_NULL_HANDLE};
    VkDescriptorSet mDummySet{VK_NULL_HANDLE};
};

#endif // BUILTIN_DESCRIPTOR_SETS_HPP