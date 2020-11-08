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

#ifndef FOE_GRAPHICS_BUILTIN_DESCRIPTOR_SETS_HPP
#define FOE_GRAPHICS_BUILTIN_DESCRIPTOR_SETS_HPP

#include <vulkan/vulkan.h>

#include <array>

enum foeBuiltinDescriptorSetLayoutFlagBits {
    FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_PROJECTION_VIEW_MATRIX = 0x00000001,
    FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_MODEL_MATRIX = 0x00000002,
    FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_MODEL_AND_BONE_STATE_MATRICES = 0x00000004,
};
using foeBuiltinDescriptorSetLayoutFlags = uint32_t;

enum foeDescriptorSetLayoutIndex {
    ProjectionViewMatrix = 0,
    ModelMatrix = 1,
    ModelAndBoneStateMatrices = 1,
    VertexShader = 2,
    TessellationControlShader = 3,
    TessellationEvaluationShader = 4,
    GeometryShader = 5,
    FragmentShader = 6,
};

class foeDescriptorSetLayoutPool;

/** Special builtin DescriptorSetLayouts and 'dummy' set
 *
 * These are core items that are integral to how most of the rendering and shared across many
 * generated shader programs.
 */
class foeBuiltinDescriptorSets {
  public:
    auto initialize(VkDevice device, foeDescriptorSetLayoutPool *pDescriptorSetLayoutPool)
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

#endif // FOE_GRAPHICS_BUILTIN_DESCRIPTOR_SETS_HPP