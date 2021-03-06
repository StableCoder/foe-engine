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

#ifndef CAMERA_SYSTEM_HPP
#define CAMERA_SYSTEM_HPP

#include <foe/graphics/session.hpp>
#include <foe/graphics/type_defs.hpp>
#include <foe/simulation/system_base.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <array>

class foePosition3dPool;
class foeCameraPool;

class foeCameraSystem : public foeSystemBase {
  public:
    VkResult initialize(foePosition3dPool *pPosition3dPool,
                        foeCameraPool *pCameraPool,
                        foeGfxSession gfxSession);
    void deinitialize();

    VkResult processCameras(uint32_t frameIndex);

  private:
    struct UniformBuffer {
        VkBuffer buffer;
        VmaAllocation alloc;
        uint32_t capacity;
    };

    // Components
    foePosition3dPool *mpPosition3dPool;
    foeCameraPool *mpCameraPool;

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