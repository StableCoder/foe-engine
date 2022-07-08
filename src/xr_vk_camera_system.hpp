// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef XR_VS_CAMERA_SYSTEM_HPP
#define XR_VS_CAMERA_SYSTEM_HPP

#include <foe/error_code.h>
#include <foe/graphics/session.h>
#include <foe/graphics/type_defs.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "xr_vk_session_view.hpp"

#include <array>
#include <vector>

class foeXrVkCameraSystem {
  public:
    foeResult initialize(foeGfxSession gfxSession);
    void deinitialize();

    foeResult processCameras(uint32_t frameIndex, std::vector<foeXrVkSessionView> &xrViews);

  private:
    struct UniformBuffer {
        VkBuffer buffer;
        VmaAllocation alloc;
        uint32_t capacity;
    };

    // Graphics
    VkDevice mDevice{VK_NULL_HANDLE};
    VmaAllocator mAllocator{VK_NULL_HANDLE};

    uint32_t mMinUniformBufferOffsetAlignment{0};

    VkDescriptorSetLayout mProjecionViewLayout{VK_NULL_HANDLE};
    uint32_t mProjectionViewBinding{0};

    std::array<UniformBuffer, FOE_GRAPHICS_MAX_BUFFERED_FRAMES> mBuffers{};
    std::array<VkDescriptorPool, FOE_GRAPHICS_MAX_BUFFERED_FRAMES> mDescriptorPools{};
};

#endif // XR_VS_CAMERA_SYSTEM_HPP