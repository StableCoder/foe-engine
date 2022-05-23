/*
    Copyright (C) 2021-2022 George Cave.

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