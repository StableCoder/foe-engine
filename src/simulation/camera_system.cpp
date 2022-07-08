// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "camera_system.hpp"

#include <foe/graphics/vk/session.hpp>
#include <foe/position/component/3d_pool.hpp>

#include "../result.h"
#include "../vk_result.h"
#include "camera.hpp"
#include "camera_pool.hpp"
#include "type_defs.h"

namespace {

glm::mat4 viewMatrix(foePosition3d const &position) noexcept {
    // Rotate * Translate
    return glm::mat4_cast(position.orientation) * glm::translate(glm::mat4(1.f), position.position);
}

} // namespace

foeResult foeCameraSystem::initialize(foePosition3dPool *pPosition3dPool,
                                      foeCameraPool *pCameraPool) {
    if (pPosition3dPool == nullptr || pCameraPool == nullptr)
        return to_foeResult(FOE_BRINGUP_INITIALIZATION_FAILED);

    // External
    mpPosition3dPool = pPosition3dPool;
    mpCameraPool = pCameraPool;

    return to_foeResult(FOE_BRINGUP_SUCCESS);
}

void foeCameraSystem::deinitialize() {
    mpCameraPool = nullptr;
    mpPosition3dPool = nullptr;
}

bool foeCameraSystem::initialized() const noexcept { return mpPosition3dPool != nullptr; }

foeResult foeCameraSystem::initializeGraphics(foeGfxSession gfxSession) {
    if (!initialized())
        return to_foeResult(FOE_BRINGUP_NOT_INITIALIZED);

    // External
    mDevice = foeGfxVkGetDevice(gfxSession);
    mAllocator = foeGfxVkGetAllocator(gfxSession);

    // Internal
    VkResult vkResult{VK_SUCCESS};

    VkPhysicalDeviceProperties devProperties;
    vkGetPhysicalDeviceProperties(foeGfxVkGetPhysicalDevice(gfxSession), &devProperties);

    mMinUniformBufferOffsetAlignment =
        std::max(static_cast<VkDeviceSize>(sizeof(glm::mat4)),
                 devProperties.limits.minUniformBufferOffsetAlignment);

    mProjecionViewLayout = foeGfxVkGetBuiltinLayout(
        gfxSession, foeBuiltinDescriptorSetLayoutFlagBits::
                        FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_PROJECTION_VIEW_MATRIX);
    mProjectionViewBinding = foeGfxVkGetBuiltinSetLayoutIndex(
        gfxSession, foeBuiltinDescriptorSetLayoutFlagBits::
                        FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_PROJECTION_VIEW_MATRIX);

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
        vkResult = vkCreateDescriptorPool(mDevice, &poolCI, nullptr, &it);
        if (vkResult != VK_SUCCESS) {
            goto INITIALIZATION_FAILED;
        }
    }

INITIALIZATION_FAILED:
    if (vkResult != VK_SUCCESS) {
        deinitialize();
    }

    return vk_to_foeResult(vkResult);
}

void foeCameraSystem::deinitializeGraphics() {
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

    mProjecionViewLayout = VK_NULL_HANDLE;

    // External
    mAllocator = VK_NULL_HANDLE;
    mDevice = VK_NULL_HANDLE;
}

bool foeCameraSystem::initializedGraphics() const noexcept { return mDevice != VK_NULL_HANDLE; }

VkResult foeCameraSystem::processCameras(uint32_t frameIndex) {
    VkResult vkResult{VK_SUCCESS};

    UniformBuffer &uniform = mBuffers[frameIndex];

    // Make sure the frame data buffer is large enough
    if (uniform.capacity < mpCameraPool->size()) {
        if (uniform.buffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(mAllocator, uniform.buffer, uniform.alloc);
        }

        VkBufferCreateInfo bufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = mMinUniformBufferOffsetAlignment * mpCameraPool->size(),
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        };

        VmaAllocationCreateInfo allocCI{
            .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
        };

        vkResult = vmaCreateBuffer(mAllocator, &bufferCI, &allocCI, &uniform.buffer, &uniform.alloc,
                                   nullptr);
        if (vkResult != VK_SUCCESS) {
            return vkResult;
        }

        uniform.capacity = mpCameraPool->size();
    }

    // Reset the DescriptorPool
    vkResetDescriptorPool(mDevice, mDescriptorPools[frameIndex], 0);

    uint8_t *pBufferData;
    VkDeviceSize offset = 0;

    vmaMapMemory(mAllocator, uniform.alloc, reinterpret_cast<void **>(&pBufferData));

    auto *pCameraData = mpCameraPool->begin<1>();

    for (auto it = mpCameraPool->begin(); it != mpCameraPool->end(); ++it, ++pCameraData) {
        auto posOffset = mpPosition3dPool->find(*it);
        if (posOffset == mpPosition3dPool->size())
            continue;

        auto *pPosition = mpPosition3dPool->begin<1>() + posOffset;

        VkDescriptorSet set;
        glm::mat4 *pMatrix = reinterpret_cast<glm::mat4 *>(pBufferData);

        *pMatrix = (*pCameraData)->projectionMatrix() * viewMatrix(*pPosition->get());

        { // Descriptor Set
            VkDescriptorSetAllocateInfo setAI{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = mDescriptorPools[frameIndex],
                .descriptorSetCount = 1U,
                .pSetLayouts = &mProjecionViewLayout,
            };

            vkResult = vkAllocateDescriptorSets(mDevice, &setAI, &set);
            if (vkResult != VK_SUCCESS) {
                return vkResult;
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

        (*pCameraData)->descriptor = set;

        pBufferData += mMinUniformBufferOffsetAlignment;
        offset += mMinUniformBufferOffsetAlignment;
    }

    vmaUnmapMemory(mAllocator, uniform.alloc);

    return vkResult;
}
