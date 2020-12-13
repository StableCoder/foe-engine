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

#include "camera_descriptor_pool.hpp"

#include "camera.hpp"

VkResult CameraDescriptorPool::initialize(foeGfxEnvironment *pGfxEnvironment,
                                          VkDescriptorSetLayout projectionViewLayout,
                                          uint32_t projectionViewBinding) {
    VkResult res;

    mDevice = pGfxEnvironment->device;
    mAllocator = pGfxEnvironment->allocator;

    mMinUniformBufferOffsetAlignment =
        std::max(static_cast<VkDeviceSize>(sizeof(glm::mat4)),
                 pGfxEnvironment->physicalDeviceLimits.minUniformBufferOffsetAlignment);

    mProjecionViewLayout = projectionViewLayout;
    mProjectionViewBinding = projectionViewBinding;

    VkDescriptorPoolSize size{
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1024,
    };

    VkDescriptorPoolCreateInfo poolCI{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 1024,
        .poolSizeCount = 1,
        .pPoolSizes = &size,
    };

    for (auto &it : mDescriptorPools) {
        res = vkCreateDescriptorPool(mDevice, &poolCI, nullptr, &it);
        if (res != VK_SUCCESS) {
            goto INITIALIZATION_FAILED;
        }
    }

INITIALIZATION_FAILED:
    if (res != VK_SUCCESS) {
        deinitialize();
    }

    return res;
}

void CameraDescriptorPool::deinitialize() {
    for (auto &it : mDescriptorPools) {
        if (it != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(mDevice, it, nullptr);
        }
        it = VK_NULL_HANDLE;
    }

    for (auto &it : mBuffers) {
        if (it.buffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(mAllocator, it.buffer, it.alloc);
        }
        it = {};
    }

    mProjecionViewLayout = VK_NULL_HANDLE;
    mAllocator = VK_NULL_HANDLE;
    mDevice = VK_NULL_HANDLE;
}

VkResult CameraDescriptorPool::generateCameraDescriptors(uint32_t frameIndex) {
    VkResult res{VK_SUCCESS};

    UniformBuffer &uniform = mBuffers[frameIndex];

    // Make sure the frame data buffer is large enough
    if (uniform.capacity < mLinkedCameras.size()) {
        if (uniform.buffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(mAllocator, uniform.buffer, uniform.alloc);
        }

        VkBufferCreateInfo bufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = mMinUniformBufferOffsetAlignment * mLinkedCameras.size(),
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        };

        VmaAllocationCreateInfo allocCI{
            .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
        };

        res = vmaCreateBuffer(mAllocator, &bufferCI, &allocCI, &uniform.buffer, &uniform.alloc,
                              nullptr);
        if (res != VK_SUCCESS) {
            return res;
        }

        uniform.capacity = mLinkedCameras.size();
    }

    // Reset the DescriptorPool
    vkResetDescriptorPool(mDevice, mDescriptorPools[frameIndex], 0);

    uint8_t *pBufferData;
    VkDeviceSize offset = 0;

    vmaMapMemory(mAllocator, uniform.alloc, reinterpret_cast<void **>(&pBufferData));

    for (auto it : mLinkedCameras) {
        VkDescriptorSet set;
        *reinterpret_cast<glm::mat4 *>(pBufferData) = it->projectionMatrix() * it->viewMatrix();

        { // Descriptor Set
            VkDescriptorSetAllocateInfo setAI{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = mDescriptorPools[frameIndex],
                .descriptorSetCount = 1U,
                .pSetLayouts = &mProjecionViewLayout,
            };

            res = vkAllocateDescriptorSets(mDevice, &setAI, &set);
            if (res != VK_SUCCESS) {
                return res;
            }

            VkDescriptorBufferInfo bufferInfo{
                .buffer = uniform.buffer,
                .offset = offset,
                .range = sizeof(glm::mat4),
            };

            VkWriteDescriptorSet writeSet{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = set,
                .dstBinding = mProjectionViewBinding,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &bufferInfo,
            };

            vkUpdateDescriptorSets(mDevice, 1, &writeSet, 0, nullptr);
        }

        it->descriptor = set;

        pBufferData += mMinUniformBufferOffsetAlignment;
        offset += mMinUniformBufferOffsetAlignment;
    }

    vmaUnmapMemory(mAllocator, uniform.alloc);

    return res;
}

void CameraDescriptorPool::linkCamera(Camera *pCamera) {
    if (pCamera->cameraDescriptorPool != nullptr) {
        return;
    }

    mLinkedCameras.emplace_back(pCamera);
    pCamera->cameraDescriptorPool = this;
}

void CameraDescriptorPool::delinkCamera(Camera *pCamera) {
    for (auto it = mLinkedCameras.begin(); it != mLinkedCameras.end(); ++it) {
        if (*it == pCamera) {
            mLinkedCameras.erase(it);
            (*it)->cameraDescriptorPool = nullptr;
            break;
        }
    }
}