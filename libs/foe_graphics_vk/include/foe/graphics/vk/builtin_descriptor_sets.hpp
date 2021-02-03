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

#ifndef FOE_GRAPHICS_VK_BUILTIN_DESCRIPTOR_SETS_HPP
#define FOE_GRAPHICS_VK_BUILTIN_DESCRIPTOR_SETS_HPP

#include <foe/graphics/builtin_descriptor_sets.hpp>
#include <foe/graphics/export.h>
#include <vulkan/vulkan.h>

class foeDescriptorSetLayoutPool;

/** Special builtin DescriptorSetLayouts and 'dummy' set
 *
 * These are core items that are integral to how most of the rendering and shared across many
 * generated shader programs.
 */
class foeBuiltinDescriptorSets {
  public:
    FOE_GFX_EXPORT auto initialize(VkDevice device,
                                   foeDescriptorSetLayoutPool *pDescriptorSetLayoutPool)
        -> VkResult;
    FOE_GFX_EXPORT void deinitialize(VkDevice device);

    FOE_GFX_EXPORT auto getBuiltinLayout(
        foeBuiltinDescriptorSetLayoutFlags builtinLayout) const noexcept -> VkDescriptorSetLayout;

    FOE_GFX_EXPORT auto getBuiltinSetLayoutIndex(
        foeBuiltinDescriptorSetLayoutFlags builtinLayout) const noexcept -> uint32_t;

    FOE_GFX_EXPORT auto getDummyLayout() const noexcept -> VkDescriptorSetLayout;
    FOE_GFX_EXPORT auto getDummySet() const noexcept -> VkDescriptorSet;

  private:
    std::array<VkDescriptorSetLayout, 3> mBuiltinLayouts;

    VkDescriptorPool mDescriptorPool{VK_NULL_HANDLE};

    VkDescriptorSetLayout mDummyLayout{VK_NULL_HANDLE};
    VkDescriptorSet mDummySet{VK_NULL_HANDLE};
};

#endif // FOE_GRAPHICS_VK_BUILTIN_DESCRIPTOR_SETS_HPP