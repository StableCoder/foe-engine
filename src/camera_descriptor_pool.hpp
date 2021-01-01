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

#ifndef CAMERA_DESCRIPTOR_POOL_HPP
#define CAMERA_DESCRIPTOR_POOL_HPP

#include <foe/graphics/device_environment.hpp>
#include <foe/graphics/type_defs.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <array>
#include <vector>

struct Camera;

class CameraDescriptorPool {
  public:
    VkResult initialize(foeVkDeviceEnvironment *pGfxEnvironment,
                        VkDescriptorSetLayout projectionViewLayout,
                        uint32_t projectionViewBinding);
    void deinitialize();

    VkResult generateCameraDescriptors(uint32_t frameIndex);

    void linkCamera(Camera *pCamera);
    void delinkCamera(Camera *pCamera);

  private:
    std::vector<Camera *> mLinkedCameras;

    struct UniformBuffer {
        VkBuffer buffer;
        VmaAllocation alloc;
        uint32_t capacity;
    };

    VkDevice mDevice{VK_NULL_HANDLE};
    VmaAllocator mAllocator{VK_NULL_HANDLE};

    uint32_t mMinUniformBufferOffsetAlignment{0};

    VkDescriptorSetLayout mProjecionViewLayout{VK_NULL_HANDLE};
    uint32_t mProjectionViewBinding{0};

    std::array<UniformBuffer, FOE_GRAPHICS_MAX_BUFFERED_FRAMES> mBuffers{};
    std::array<VkDescriptorPool, FOE_GRAPHICS_MAX_BUFFERED_FRAMES> mDescriptorPools{};
};

#endif // CAMERA_DESCRIPTOR_POOL_HPP