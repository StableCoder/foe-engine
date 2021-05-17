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

#ifndef POSITION_DESCRIPTOR_POOL_HPP
#define POSITION_DESCRIPTOR_POOL_HPP

#include <foe/data_pool.hpp>
#include <foe/ecs/id.hpp>
#include <foe/graphics/session.hpp>
#include <foe/graphics/type_defs.hpp>
#include <foe/position/3d.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <array>
#include <memory>
#include <vector>

struct PositionDescriptorPool {
  public:
    VkResult initialize(foeGfxSession gfxSession,
                        VkDescriptorSetLayout modelMatrixLayout,
                        uint32_t modelMatrixBinding);
    void deinitialize();

    VkResult generatePositionDescriptors(
        uint32_t frameIndex, foeDataPool<foeEntityID, std::unique_ptr<Position3D>> &positionPool);

  private:
    struct UniformBuffer {
        VkBuffer buffer;
        VmaAllocation alloc;
        uint32_t capacity;
    };

    VkDevice mDevice{VK_NULL_HANDLE};
    VmaAllocator mAllocator{VK_NULL_HANDLE};

    uint32_t mMinUniformBufferOffsetAlignment{0};

    VkDescriptorSetLayout mModelMatrixLayout{VK_NULL_HANDLE};
    uint32_t mModelMatrixBinding{0};

    std::array<UniformBuffer, FOE_GRAPHICS_MAX_BUFFERED_FRAMES> mBuffers{};
    std::array<VkDescriptorPool, FOE_GRAPHICS_MAX_BUFFERED_FRAMES> mDescriptorPools{};
};

#endif // POSITION_DESCRIPTOR_POOL_HPP