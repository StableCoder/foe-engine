// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef CAMERA_SYSTEM_HPP
#define CAMERA_SYSTEM_HPP

#include <foe/error_code.h>
#include <foe/graphics/session.h>
#include <foe/graphics/type_defs.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <array>

class foePosition3dPool;
class foeCameraPool;

class foeCameraSystem {
  public:
    foeResultSet initialize(foePosition3dPool *pPosition3dPool, foeCameraPool *pCameraPool);
    void deinitialize();
    bool initialized() const noexcept;

    foeResultSet initializeGraphics(foeGfxSession gfxSession);
    void deinitializeGraphics();
    bool initializedGraphics() const noexcept;

    VkResult processCameras(uint32_t frameIndex);

  private:
    struct UniformBuffer {
        VkBuffer buffer;
        VmaAllocation alloc;
        uint32_t capacity;
    };

    // Components
    foePosition3dPool *mpPosition3dPool{nullptr};
    foeCameraPool *mpCameraPool{nullptr};

    // Graphics
    VkDevice mDevice{VK_NULL_HANDLE};
    VmaAllocator mAllocator{VK_NULL_HANDLE};

    uint32_t mMinUniformBufferOffsetAlignment{0};

    VkDescriptorSetLayout mProjecionViewLayout{VK_NULL_HANDLE};
    uint32_t mProjectionViewBinding{0};

    std::array<UniformBuffer, FOE_GRAPHICS_MAX_BUFFERED_FRAMES> mBuffers{};
    std::array<VkDescriptorPool, FOE_GRAPHICS_MAX_BUFFERED_FRAMES> mDescriptorPools{};
};

#endif // CAMERA_SYSTEM_HPP