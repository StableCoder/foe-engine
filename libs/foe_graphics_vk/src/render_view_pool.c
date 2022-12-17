// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/render_view_pool.h>
#include <foe/graphics/vk/render_view_pool.h>

#include <foe/graphics/builtin_descriptor_sets.h>
#include <foe/graphics/type_defs.h>
#include <foe/graphics/vk/session.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "result.h"
#include "vk_result.h"

#include <stdlib.h>
#include <string.h>

size_t const cViewDataSize = sizeof(float) * 16;

typedef struct foeGfxVkRenderView {
    void *pBuffer;
    uint32_t bufferSize;
    VkDescriptorSet descriptorSets[FOE_GRAPHICS_MAX_BUFFERED_FRAMES];
} foeGfxVkRenderView;

FOE_DEFINE_HANDLE_CASTS(render_view, foeGfxVkRenderView, foeGfxRenderView)

typedef struct foeGfxVkRenderViewPool {
    uint32_t viewCount;

    void *pSimulationBuffer;
    uint32_t simulationBufferSize;

    foeGfxVkRenderView *pRenderViews;

    VkDescriptorPool descriptorPool;

    uint32_t currentBufferIndex;
    VkBuffer buffers[FOE_GRAPHICS_MAX_BUFFERED_FRAMES];
    VmaAllocation bufferAllocations[FOE_GRAPHICS_MAX_BUFFERED_FRAMES];
} foeGfxVkRenderViewPool;

FOE_DEFINE_HANDLE_CASTS(render_view_pool, foeGfxVkRenderViewPool, foeGfxRenderViewPool)

foeResultSet foeGfxCreateRenderViewPool(foeGfxSession session,
                                        uint32_t viewCount,
                                        foeGfxRenderViewPool *pRenderViewPool) {
    foeResultSet result = to_foeResult(FOE_GRAPHICS_VK_SUCCESS);

    // Allocate pool
    foeGfxVkRenderViewPool *pNewRenderViewPool = calloc(1, sizeof(foeGfxVkRenderViewPool));
    if (pNewRenderViewPool == NULL)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_OUT_OF_MEMORY);

    // Set basic values
    pNewRenderViewPool->viewCount = viewCount;

    // Allocate simulation buffer
    VkPhysicalDeviceProperties devProperties;
    vkGetPhysicalDeviceProperties(foeGfxVkGetPhysicalDevice(session), &devProperties);

    uint32_t minBufferAlignment = cViewDataSize;
    if (devProperties.limits.minUniformBufferOffsetAlignment > minBufferAlignment)
        minBufferAlignment = devProperties.limits.minUniformBufferOffsetAlignment;

    pNewRenderViewPool->simulationBufferSize = viewCount * minBufferAlignment;
    pNewRenderViewPool->pSimulationBuffer = malloc(pNewRenderViewPool->simulationBufferSize);
    if (pNewRenderViewPool->pSimulationBuffer == NULL) {
        result = to_foeResult(FOE_GRAPHICS_VK_ERROR_OUT_OF_MEMORY);
        goto CREATE_FAILED;
    }

    // Allocate render views
    pNewRenderViewPool->pRenderViews = calloc(viewCount, sizeof(foeGfxVkRenderView));
    if (pNewRenderViewPool->pRenderViews == NULL) {
        result = to_foeResult(FOE_GRAPHICS_VK_ERROR_OUT_OF_MEMORY);
        goto CREATE_FAILED;
    }

    { // Graphics
        VkDescriptorSetLayout projectionViewLayout = foeGfxVkGetBuiltinLayout(
            session, FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_PROJECTION_VIEW_MATRIX);

        uint32_t projectionViewBinding = foeGfxVkGetBuiltinSetLayoutIndex(
            session, FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_PROJECTION_VIEW_MATRIX);

        // Create Descriptor pool
        VkDescriptorPoolSize descriptorPoolSize = {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = viewCount * FOE_GRAPHICS_MAX_BUFFERED_FRAMES,
        };

        VkDescriptorPoolCreateInfo descriptorPoolCI = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = viewCount * FOE_GRAPHICS_MAX_BUFFERED_FRAMES,
            .poolSizeCount = 1,
            .pPoolSizes = &descriptorPoolSize,
        };

        VkResult vkResult = vkCreateDescriptorPool(foeGfxVkGetDevice(session), &descriptorPoolCI,
                                                   NULL, &pNewRenderViewPool->descriptorPool);
        if (vkResult != VK_SUCCESS) {
            result = vk_to_foeResult(vkResult);
            goto CREATE_FAILED;
        }

        // Create Graphics Buffers
        VkBufferCreateInfo bufferCI = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = minBufferAlignment * viewCount,
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        };

        VmaAllocationCreateInfo allocCI = {
            .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
        };

        for (size_t i = 0; i < FOE_GRAPHICS_MAX_BUFFERED_FRAMES; ++i) {
            vkResult = vmaCreateBuffer(foeGfxVkGetAllocator(session), &bufferCI, &allocCI,
                                       &pNewRenderViewPool->buffers[i],
                                       &pNewRenderViewPool->bufferAllocations[i], NULL);
            if (vkResult != VK_SUCCESS) {
                result = vk_to_foeResult(vkResult);
                goto CREATE_FAILED;
            }
        }

        // Allocate DescriptorSets and set them in RenderViews
        VkDeviceSize viewMemoryOffset = 0;

        VkDescriptorSetAllocateInfo descriptorSetAI = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = pNewRenderViewPool->descriptorPool,
            .descriptorSetCount = 1U,
            .pSetLayouts = &projectionViewLayout,
        };

        for (uint32_t i = 0; i < viewCount; ++i) {
            // Set the pointer to the simulation data buffer
            pNewRenderViewPool->pRenderViews[i].pBuffer =
                ((uint8_t *)pNewRenderViewPool->pSimulationBuffer) + (minBufferAlignment * i);

            VkDescriptorBufferInfo bufferInfo = {
                .offset = viewMemoryOffset,
                .range = cViewDataSize,
            };
            viewMemoryOffset += minBufferAlignment;

            VkWriteDescriptorSet writeSet = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstBinding = projectionViewBinding,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &bufferInfo,
            };

            for (uint32_t j = 0; j < FOE_GRAPHICS_MAX_BUFFERED_FRAMES; ++j) {
                vkResult = vkAllocateDescriptorSets(
                    foeGfxVkGetDevice(session), &descriptorSetAI,
                    pNewRenderViewPool->pRenderViews[i].descriptorSets + j);
                if (vkResult != VK_SUCCESS) {
                    result = vk_to_foeResult(vkResult);
                    goto CREATE_FAILED;
                }

                bufferInfo.buffer = pNewRenderViewPool->buffers[j];
                writeSet.dstSet = pNewRenderViewPool->pRenderViews[i].descriptorSets[j];

                vkUpdateDescriptorSets(foeGfxVkGetDevice(session), 1U, &writeSet, 0, NULL);
            }
        }
    }

CREATE_FAILED:
    if (result.value == FOE_SUCCESS) {
        *pRenderViewPool = render_view_pool_to_handle(pNewRenderViewPool);
    } else {
        foeGfxDestroyRenderViewPool(session, render_view_pool_to_handle(pNewRenderViewPool));
    }

    return result;
}

void foeGfxDestroyRenderViewPool(foeGfxSession session, foeGfxRenderViewPool renderViewPool) {
    foeGfxVkRenderViewPool *pRenderViewPool = render_view_pool_from_handle(renderViewPool);

    // Destroy Graphics Buffers
    for (size_t i = 0; i < FOE_GRAPHICS_MAX_BUFFERED_FRAMES; ++i) {
        if (pRenderViewPool->buffers[i] != VK_NULL_HANDLE)
            vmaDestroyBuffer(foeGfxVkGetAllocator(session), pRenderViewPool->buffers[i],
                             pRenderViewPool->bufferAllocations[i]);
    }

    // Descriptor Pool
    if (pRenderViewPool->descriptorPool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(foeGfxVkGetDevice(session), pRenderViewPool->descriptorPool, NULL);

    // Render Views
    if (pRenderViewPool->pRenderViews != NULL)
        free(pRenderViewPool->pRenderViews);

    // Simulation Buffer
    if (pRenderViewPool->pSimulationBuffer != NULL)
        free(pRenderViewPool->pSimulationBuffer);

    free(pRenderViewPool);
}

foeResultSet foeGfxUpdateRenderViewPool(foeGfxSession session,
                                        foeGfxRenderViewPool renderViewPool) {
    foeGfxVkRenderViewPool *pRenderViewPool = render_view_pool_from_handle(renderViewPool);

    uint32_t nextBufferIndex = pRenderViewPool->currentBufferIndex + 1;
    if (nextBufferIndex >= FOE_GRAPHICS_MAX_BUFFERED_FRAMES)
        nextBufferIndex = 0;

    void *pMappedData;
    VkResult vkResult =
        vmaMapMemory(foeGfxVkGetAllocator(session),
                     pRenderViewPool->bufferAllocations[nextBufferIndex], &pMappedData);
    if (vkResult != VK_SUCCESS)
        return vk_to_foeResult(vkResult);

    memcpy(pMappedData, pRenderViewPool->pSimulationBuffer, pRenderViewPool->simulationBufferSize);

    vmaUnmapMemory(foeGfxVkGetAllocator(session),
                   pRenderViewPool->bufferAllocations[nextBufferIndex]);

    pRenderViewPool->currentBufferIndex = nextBufferIndex;

    return to_foeResult(FOE_GRAPHICS_VK_SUCCESS);
}

foeResultSet foeGfxAllocateRenderView(foeGfxRenderViewPool renderViewPool,
                                      foeGfxRenderView *pRenderView) {
    foeGfxVkRenderViewPool *pRenderViewPool = render_view_pool_from_handle(renderViewPool);

    for (uint32_t i = 0; i < pRenderViewPool->viewCount; ++i) {
        foeGfxVkRenderView *pNewRenderView = pRenderViewPool->pRenderViews + i;

        if (pNewRenderView->bufferSize == 0) {
            // Not actively used yet, return it
            pNewRenderView->bufferSize = cViewDataSize;

            *pRenderView = render_view_to_handle(pNewRenderView);
            return to_foeResult(FOE_GRAPHICS_VK_SUCCESS);
        }
    }

    return to_foeResult(FOE_GRAPHICS_VK_ERROR_OUT_OF_POOL_MEMORY);
}

void foeGfxFreeRenderView(foeGfxRenderView renderView) {
    foeGfxVkRenderView *pRenderView = render_view_from_handle(renderView);

    pRenderView->bufferSize = 0;
}

foeResultSet foeGfxUpdateRenderView(foeGfxRenderView renderView, uint32_t dataSize, void *pData) {
    foeGfxVkRenderView *pRenderView = render_view_from_handle(renderView);

    if (dataSize > pRenderView->bufferSize)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_DATA_LARGER_THAN_BUFFER);

    memcpy(pRenderView->pBuffer, pData, dataSize);

    return to_foeResult(FOE_GRAPHICS_VK_SUCCESS);
}

VkDescriptorSet foeGfxVkGetRenderViewDescriptorSet(foeGfxRenderViewPool renderViewPool,
                                                   foeGfxRenderView renderView) {
    foeGfxVkRenderViewPool *pRenderViewPool = render_view_pool_from_handle(renderViewPool);
    foeGfxVkRenderView *pRenderView = render_view_from_handle(renderView);

    return pRenderView->descriptorSets[pRenderViewPool->currentBufferIndex];
}