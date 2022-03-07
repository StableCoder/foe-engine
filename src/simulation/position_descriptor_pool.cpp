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

#include "position_descriptor_pool.hpp"

#include <foe/graphics/vk/session.hpp>
#include <foe/position/component/3d_pool.hpp>
#include <vk_error_code.hpp>
#include <vulkan/vulkan_core.h>

#include "../error_code.hpp"
#include "type_defs.h"

auto PositionDescriptorPool::initialize(foePosition3dPool *pPosition3dPool) -> std::error_code {
    if (pPosition3dPool == nullptr) {
        return FOE_BRINGUP_INITIALIZATION_FAILED;
    }

    mpPosition3dPool = pPosition3dPool;

    return VK_SUCCESS;
}

void PositionDescriptorPool::deinitialize() { mpPosition3dPool = nullptr; }

bool PositionDescriptorPool::initialized() const noexcept { return mpPosition3dPool != nullptr; }

auto PositionDescriptorPool::initializeGraphics(foeGfxSession gfxSession) -> std::error_code {
    if (!initialized())
        return FOE_BRINGUP_NOT_INITIALIZED;

    // External
    mDevice = foeGfxVkGetDevice(gfxSession);
    mAllocator = foeGfxVkGetAllocator(gfxSession);

    // Internal
    VkResult res{VK_SUCCESS};

    VkPhysicalDeviceProperties devProperties;
    vkGetPhysicalDeviceProperties(foeGfxVkGetPhysicalDevice(gfxSession), &devProperties);

    mMinUniformBufferOffsetAlignment =
        std::max(static_cast<VkDeviceSize>(sizeof(glm::mat4)),
                 devProperties.limits.minUniformBufferOffsetAlignment);

    mModelMatrixLayout = foeGfxVkGetBuiltinLayout(
        gfxSession,
        foeBuiltinDescriptorSetLayoutFlagBits::FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_MODEL_MATRIX);
    mModelMatrixBinding = foeGfxVkGetBuiltinSetLayoutIndex(
        gfxSession,
        foeBuiltinDescriptorSetLayoutFlagBits::FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_MODEL_MATRIX);

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

void PositionDescriptorPool::deinitializeGraphics() {
    // Internal
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

    // External
    mModelMatrixLayout = VK_NULL_HANDLE;
    mAllocator = VK_NULL_HANDLE;
    mDevice = VK_NULL_HANDLE;
}

bool PositionDescriptorPool::initializedGraphics() const noexcept {
    return mDevice != VK_NULL_HANDLE;
}

VkResult PositionDescriptorPool::generatePositionDescriptors(uint32_t frameIndex) {
    VkResult res{VK_SUCCESS};

    UniformBuffer &uniform = mBuffers[frameIndex];

    // Make sure the frame data buffer is large enough
    if (uniform.capacity < mpPosition3dPool->size()) {
        if (uniform.buffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(mAllocator, uniform.buffer, uniform.alloc);
        }

        VkBufferCreateInfo bufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = mMinUniformBufferOffsetAlignment * mpPosition3dPool->size(),
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

        uniform.capacity = mpPosition3dPool->size();
    }

    // Reset the DescriptorPool
    vkResetDescriptorPool(mDevice, mDescriptorPools[frameIndex], 0);

    // Generate the new data
    uint8_t *pBufferData;
    VkDeviceSize offset = 0;

    vmaMapMemory(mAllocator, uniform.alloc, reinterpret_cast<void **>(&pBufferData));

    auto *const pDataEnd = mpPosition3dPool->end<1>();

    for (auto *pData = mpPosition3dPool->begin<1>(); pData != pDataEnd; ++pData) {
        VkDescriptorSet set;
        *reinterpret_cast<glm::mat4 *>(pBufferData) =
            glm::mat4_cast(pData->get()->orientation) *
            glm::translate(glm::mat4(1.f), pData->get()->position);

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

        pData->get()->descriptorSet = set;

        pBufferData += mMinUniformBufferOffsetAlignment;
        offset += mMinUniformBufferOffsetAlignment;
    }

    vmaUnmapMemory(mAllocator, uniform.alloc);

    return res;
}