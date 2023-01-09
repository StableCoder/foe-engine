// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef POSITION_DESCRIPTOR_POOL_HPP
#define POSITION_DESCRIPTOR_POOL_HPP

#include <foe/ecs/id.h>
#include <foe/graphics/session.h>
#include <foe/graphics/type_defs.h>
#include <foe/position/component/3d_pool.h>
#include <foe/result.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <array>
#include <memory>
#include <vector>

struct PositionDescriptorPool {
  public:
    foeResultSet initialize(foePosition3dPool position3dPool);
    void deinitialize();
    bool initialized() const noexcept;

    foeResultSet initializeGraphics(foeGfxSession gfxSession);
    void deinitializeGraphics();
    bool initializedGraphics() const noexcept;

    VkResult generatePositionDescriptors(uint32_t frameIndex);

  private:
    struct UniformBuffer {
        VkBuffer buffer;
        VmaAllocation alloc;
        uint32_t capacity;
    };

    // Components
    foePosition3dPool mPosition3dPool{FOE_NULL_HANDLE};

    // Graphics
    VkDevice mDevice{VK_NULL_HANDLE};
    VmaAllocator mAllocator{VK_NULL_HANDLE};

    uint32_t mMinUniformBufferOffsetAlignment{0};

    VkDescriptorSetLayout mModelMatrixLayout{VK_NULL_HANDLE};
    uint32_t mModelMatrixBinding{0};

    std::array<UniformBuffer, FOE_GRAPHICS_MAX_BUFFERED_FRAMES> mBuffers{};
    std::array<VkDescriptorPool, FOE_GRAPHICS_MAX_BUFFERED_FRAMES> mDescriptorPools{};
};

#endif // POSITION_DESCRIPTOR_POOL_HPP