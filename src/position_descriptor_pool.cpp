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

#include "position_descriptor_pool.hpp"

#include <foe/graphics/vk/session.hpp>

#include "position_3d.hpp"

VkResult PositionDescriptorPool::initialize(foeGfxSession session,
                                            VkDescriptorSetLayout modelMatrixLayout,
                                            uint32_t modelMatrixBinding) {
    VkResult res;

    mDevice = foeGfxVkGetDevice(session);
    mAllocator = foeGfxVkGetAllocator(session);

    VkPhysicalDeviceProperties devProperties;
    vkGetPhysicalDeviceProperties(foeGfxVkGetPhysicalDevice(session), &devProperties);

    mMinUniformBufferOffsetAlignment =
        std::max(static_cast<VkDeviceSize>(sizeof(glm::mat4)),
                 devProperties.limits.minUniformBufferOffsetAlignment);

    mModelMatrixLayout = modelMatrixLayout;
    mModelMatrixBinding = modelMatrixBinding;

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

void PositionDescriptorPool::deinitialize() {
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

    mModelMatrixLayout = VK_NULL_HANDLE;
    mAllocator = VK_NULL_HANDLE;
    mDevice = VK_NULL_HANDLE;
}

VkResult PositionDescriptorPool::generatePositionDescriptors(
    uint32_t frameIndex, std::map<foeEntityID, std::unique_ptr<Position3D>> &positions) {
    VkResult res{VK_SUCCESS};

    UniformBuffer &uniform = mBuffers[frameIndex];

    // Make sure the frame data buffer is large enough
    if (uniform.capacity < positions.size()) {
        if (uniform.buffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(mAllocator, uniform.buffer, uniform.alloc);
        }

        VkBufferCreateInfo bufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = mMinUniformBufferOffsetAlignment * positions.size(),
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

        uniform.capacity = positions.size();
    }

    // Reset the DescriptorPool
    vkResetDescriptorPool(mDevice, mDescriptorPools[frameIndex], 0);

    // Generate the new data
    uint8_t *pBufferData;
    VkDeviceSize offset = 0;

    vmaMapMemory(mAllocator, uniform.alloc, reinterpret_cast<void **>(&pBufferData));

    for (auto const &it : positions) {
        VkDescriptorSet set;
        *reinterpret_cast<glm::mat4 *>(pBufferData) =
            glm::mat4_cast(it.second->orientation) *
            glm::translate(glm::mat4(1.f), it.second->position);

        { // Descriptor Set
            VkDescriptorSetAllocateInfo setAI{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = mDescriptorPools[frameIndex],
                .descriptorSetCount = 1U,
                .pSetLayouts = &mModelMatrixLayout,
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
                .dstBinding = mModelMatrixBinding,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &bufferInfo,
            };

            vkUpdateDescriptorSets(mDevice, 1, &writeSet, 0, nullptr);
        }

        it.second->descriptorSet = set;

        pBufferData += mMinUniformBufferOffsetAlignment;
        offset += mMinUniformBufferOffsetAlignment;
    }

    vmaUnmapMemory(mAllocator, uniform.alloc);

    return res;
}