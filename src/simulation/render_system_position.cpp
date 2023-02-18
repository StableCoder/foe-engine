// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "render_system_position.hpp"

#include <foe/graphics/vk/session.h>
#include <foe/memory_alignment.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "../result.h"
#include "../vk_result.h"

#include <algorithm>
#include <cstdlib>

size_t constexpr cPositionDataSize = sizeof(glm::mat4);

namespace {

void destroyPositionGpuData(foeGfxSession gfxSession, RenderSystemPositionGpuData const *pData) {
    free(pData->pDescriptorSets);

    if (pData->descriptorPool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(foeGfxVkGetDevice(gfxSession), pData->descriptorPool, nullptr);

    if (pData->buffer != VK_NULL_HANDLE)
        vmaDestroyBuffer(foeGfxVkGetAllocator(gfxSession), pData->buffer, pData->alloc);
}

[[nodiscard]] foeResultSet createPositionGpuData(foeGfxSession gfxSession,
                                                 uint32_t alignment,
                                                 uint32_t capacity,
                                                 VkDescriptorSetLayout layout,
                                                 uint32_t binding,
                                                 RenderSystemPositionGpuData *pData) {
    RenderSystemPositionGpuData newData = {
        .itemCapacity = capacity,
    };
    foeResultSet result = to_foeResult(FOE_BRINGUP_SUCCESS);
    VkResult vkResult = VK_SUCCESS;

    { // GPU Buffer
        VkBufferCreateInfo bufferCI = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = capacity * alignment,
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        };

        VmaAllocationCreateInfo allocCI = {
            .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
        };

        vkResult = vmaCreateBuffer(foeGfxVkGetAllocator(gfxSession), &bufferCI, &allocCI,
                                   &newData.buffer, &newData.alloc, NULL);
        if (vkResult != VK_SUCCESS)
            goto CREATE_POSITION_GPU_DATA_FAILED;
    }

    { // Descriptor Pool
        VkDescriptorPoolSize descriptorPoolSize = {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = capacity,
        };

        VkDescriptorPoolCreateInfo descriptorPoolCI = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = capacity,
            .poolSizeCount = 1,
            .pPoolSizes = &descriptorPoolSize,
        };
        vkResult = vkCreateDescriptorPool(foeGfxVkGetDevice(gfxSession), &descriptorPoolCI, NULL,
                                          &newData.descriptorPool);
        if (vkResult != VK_SUCCESS)
            goto CREATE_POSITION_GPU_DATA_FAILED;
    }

    { // Descriptor Sets
        newData.pDescriptorSets = (VkDescriptorSet *)malloc(capacity * sizeof(VkDescriptorSet));
        if (newData.pDescriptorSets == nullptr) {
            result = to_foeResult(FOE_BRINGUP_ERROR_OUT_OF_MEMORY);
            goto CREATE_POSITION_GPU_DATA_FAILED;
        }

        VkDescriptorSetAllocateInfo descriptorSetAI = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = newData.descriptorPool,
            .descriptorSetCount = 1U,
            .pSetLayouts = &layout,
        };

        VkDescriptorBufferInfo bufferInfo = {
            .buffer = newData.buffer,
            .range = cPositionDataSize,
        };

        VkWriteDescriptorSet writeSet = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstBinding = binding,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pBufferInfo = &bufferInfo,
        };

        for (uint32_t i = 0; i < capacity; ++i) {
            vkResult = vkAllocateDescriptorSets(foeGfxVkGetDevice(gfxSession), &descriptorSetAI,
                                                &newData.pDescriptorSets[i]);
            if (vkResult != VK_SUCCESS)
                goto CREATE_POSITION_GPU_DATA_FAILED;

            bufferInfo.offset = i * alignment;
            writeSet.dstSet = newData.pDescriptorSets[i];

            vkUpdateDescriptorSets(foeGfxVkGetDevice(gfxSession), 1, &writeSet, 0, nullptr);
        }
    }

CREATE_POSITION_GPU_DATA_FAILED:
    if (vkResult != VK_SUCCESS)
        result = vk_to_foeResult(vkResult);

    if (result.value == FOE_SUCCESS) {
        *pData = newData;
    } else {
        destroyPositionGpuData(gfxSession, &newData);
    }

    return result;
}

} // namespace

foeResultSet initializePositionData(foeGfxSession gfxSession,
                                    RenderSystemPositionData &positionData) {

    // Need to figure out the minimum alignment supported by the renderer
    VkPhysicalDeviceProperties devProperties;
    vkGetPhysicalDeviceProperties(foeGfxVkGetPhysicalDevice(gfxSession), &devProperties);

    positionData = {
        .positionLayout = foeGfxVkGetBuiltinLayout(
            gfxSession,
            foeBuiltinDescriptorSetLayoutFlagBits::FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_MODEL_MATRIX),
        .positionBinding = foeGfxVkGetBuiltinSetLayoutIndex(
            gfxSession,
            foeBuiltinDescriptorSetLayoutFlagBits::FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_MODEL_MATRIX),
        .alignment = foeGetAlignedSize(devProperties.limits.minUniformBufferOffsetAlignment,
                                       cPositionDataSize),
    };

    return to_foeResult(FOE_BRINGUP_SUCCESS);
}

void deinitializePositionData(foeGfxSession gfxSession, RenderSystemPositionData &positionData) {
    for (size_t i = 0; i < FOE_GRAPHICS_MAX_BUFFERED_FRAMES; ++i) {
        RenderSystemPositionGpuData &gfxData = positionData.gpuData[i];

        destroyPositionGpuData(gfxSession, positionData.gpuData + i);
    }

    free(positionData.pCpuBuffer);
}

foeResultSet insertPositionData(RenderSystemPositionData &positionData,
                                size_t index,
                                foePosition3d const *pPositionData) {
    size_t newCapacity = positionData.itemCapacity;
    if (positionData.itemCount >= positionData.itemCapacity)
        // The number of items is about to go over capacity, increase it to double the current count
        newCapacity = positionData.itemCount * 2;
    if (newCapacity == 0)
        newCapacity = 1;

    if (newCapacity != positionData.itemCapacity) {
        void *pNewBuffer = malloc(newCapacity * positionData.alignment);
        if (pNewBuffer == nullptr)
            return to_foeResult(FOE_BRINGUP_ERROR_OUT_OF_MEMORY);

        memcpy(pNewBuffer, positionData.pCpuBuffer,
               positionData.itemCount * positionData.alignment);

        free(positionData.pCpuBuffer);
        positionData.pCpuBuffer = pNewBuffer;
        positionData.itemCapacity = newCapacity;
    }

    uint8_t *pDataBuffer = (uint8_t *)positionData.pCpuBuffer;
    pDataBuffer += index * positionData.alignment;

    // Shift rest of data to the right
    memmove(pDataBuffer + positionData.alignment, pDataBuffer,
            (positionData.itemCount - index) * positionData.alignment);

    glm::mat4 *pMatrix = (glm::mat4 *)pDataBuffer;

    *pMatrix = glm::mat4_cast(pPositionData->orientation) *
               glm::translate(glm::mat4(1.f), pPositionData->position);

    ++positionData.itemCount;

    return to_foeResult(FOE_BRINGUP_SUCCESS);
}

void removePositionData(RenderSystemPositionData &positionData, size_t index) {
    uint8_t *pDataBuffer = (uint8_t *)positionData.pCpuBuffer;
    pDataBuffer += index * positionData.alignment;

    // Shift other remaining items to the left by one
    memmove(pDataBuffer, pDataBuffer + positionData.alignment,
            (positionData.itemCount - index - 1) * positionData.alignment);

    --positionData.itemCount;
}

[[nodiscard]] foeResultSet preparePositionGpuData(RenderSystemPositionData &positionData,
                                                  foeGfxSession gfxSession,
                                                  uint32_t frameIndex) {
    foeResultSet result = to_foeResult(FOE_BRINGUP_SUCCESS);

    if (positionData.itemCount > 0) {
        if (positionData.gpuData[frameIndex].itemCapacity < positionData.itemCapacity) {
            destroyPositionGpuData(gfxSession, &positionData.gpuData[frameIndex]);
            positionData.gpuData[frameIndex] = {};

            result =
                createPositionGpuData(gfxSession, positionData.alignment, positionData.itemCapacity,
                                      positionData.positionLayout, positionData.positionBinding,
                                      &positionData.gpuData[frameIndex]);
            if (result.value != FOE_SUCCESS)
                return result;
        }

        void *pMappedData;
        VkResult vkResult = vmaMapMemory(foeGfxVkGetAllocator(gfxSession),
                                         positionData.gpuData[frameIndex].alloc, &pMappedData);
        if (vkResult != VK_SUCCESS)
            return vk_to_foeResult(vkResult);

        memcpy(pMappedData, positionData.pCpuBuffer,
               positionData.itemCapacity * positionData.alignment);

        vmaUnmapMemory(foeGfxVkGetAllocator(gfxSession), positionData.gpuData[frameIndex].alloc);
    }

    return result;
}