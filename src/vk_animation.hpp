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

#ifndef VK_ANIMATION_HPP
#define VK_ANIMATION_HPP

#include <foe/graphics/session.hpp>
#include <foe/graphics/type_defs.hpp>
#include <foe/model/animation.hpp>
#include <foe/model/armature.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <array>

class VkAnimationPool {
  public:
    VkResult initialize(foeGfxSession gfxSession,
                        VkDescriptorSetLayout boneSetLayout,
                        uint32_t boneSetBinding);
    void deinitialize();

    VkResult generateBoneAnimation(uint32_t frameIndex,
                                   double time,
                                   std::vector<foeMeshBone> const &bones,
                                   foeAnimation const *pAnimation);

  public:
    struct UniformBuffer {
        VkBuffer buffer;
        VmaAllocation alloc;
        uint32_t capacity;
    };

    VkDevice mDevice{VK_NULL_HANDLE};
    VmaAllocator mAllocator{VK_NULL_HANDLE};

    VkDescriptorSetLayout mBoneSetLayout{VK_NULL_HANDLE};
    uint32_t mBoneSetBinding;

    VkDescriptorSet mSet{VK_NULL_HANDLE};

    uint32_t maxBones{64};

    uint32_t mMinUniformBufferOffsetAlignment{0};

    std::array<UniformBuffer, FOE_GRAPHICS_MAX_BUFFERED_FRAMES> mModelBuffers{};
    std::array<VkDescriptorPool, FOE_GRAPHICS_MAX_BUFFERED_FRAMES> mModelDescriptorPools{};

    std::array<UniformBuffer, FOE_GRAPHICS_MAX_BUFFERED_FRAMES> mBoneBuffers{};
    std::array<VkDescriptorPool, FOE_GRAPHICS_MAX_BUFFERED_FRAMES> mBoneDescriptorPools{};
};

#endif // VK_ANIMATION_HPP