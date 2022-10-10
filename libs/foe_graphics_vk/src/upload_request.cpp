// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "upload_request.hpp"

#include "result.h"
#include "vk_result.h"

#include <new>

foeResultSet foeGfxVkCreateUploadRequest(VkDevice device,
                                         VkCommandPool srcCommandPool,
                                         VkCommandPool dstCommandPool,
                                         foeGfxVkUploadRequest **pUploadRequest) {

    auto *pNewUploadRequest = new (std::nothrow) foeGfxVkUploadRequest{
        .device = device,
        .srcCmdPool = srcCommandPool,
        .dstCmdPool = dstCommandPool,
    };
    if (pNewUploadRequest == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_OUT_OF_MEMORY);

    VkCommandBufferAllocateInfo bufferAI{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = pNewUploadRequest->dstCmdPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkFenceCreateInfo fenceCI{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };

    // Destination
    VkResult vkResult =
        vkAllocateCommandBuffers(device, &bufferAI, &pNewUploadRequest->dstCmdBuffer);
    if (vkResult != VK_SUCCESS) {
        goto CREATE_FAILED;
    }

    vkResult = vkCreateFence(device, &fenceCI, nullptr, &pNewUploadRequest->dstFence);
    if (vkResult != VK_SUCCESS) {
        goto CREATE_FAILED;
    }

    // Source
    if (pNewUploadRequest->srcCmdPool != VK_NULL_HANDLE) {
        bufferAI.commandPool = pNewUploadRequest->srcCmdPool;

        vkResult = vkAllocateCommandBuffers(device, &bufferAI, &pNewUploadRequest->srcCmdBuffer);
        if (vkResult != VK_SUCCESS) {
            goto CREATE_FAILED;
        }

        vkResult = vkCreateFence(device, &fenceCI, nullptr, &pNewUploadRequest->srcFence);
        if (vkResult != VK_SUCCESS) {
            goto CREATE_FAILED;
        }

        { // Only need semaphore to synchronize if going across queues
            VkSemaphoreCreateInfo semaphoreCI{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            };

            vkResult =
                vkCreateSemaphore(device, &semaphoreCI, nullptr, &pNewUploadRequest->copyComplete);
            if (vkResult != VK_SUCCESS) {
                goto CREATE_FAILED;
            }
        }
    }

CREATE_FAILED:
    if (vkResult == VK_SUCCESS) {
        *pUploadRequest = pNewUploadRequest;
    } else {
        foeGfxVkDestroyUploadRequest(device, pNewUploadRequest);
    }

    return vk_to_foeResult(vkResult);
}

void foeGfxVkDestroyUploadRequest(VkDevice device, foeGfxVkUploadRequest *pUploadRequest) {
    if (pUploadRequest->copyComplete != VK_NULL_HANDLE) {
        vkDestroySemaphore(device, pUploadRequest->copyComplete, nullptr);
    }

    if (pUploadRequest->dstFence != VK_NULL_HANDLE) {
        vkDestroyFence(device, pUploadRequest->dstFence, nullptr);
    }

    if (pUploadRequest->dstCmdBuffer != VK_NULL_HANDLE) {
        // @todo Synchronize command pool
        vkFreeCommandBuffers(device, pUploadRequest->dstCmdPool, 1, &pUploadRequest->dstCmdBuffer);
    }

    if (pUploadRequest->srcFence != VK_NULL_HANDLE) {
        vkDestroyFence(device, pUploadRequest->srcFence, nullptr);
    }

    if (pUploadRequest->srcCmdBuffer != VK_NULL_HANDLE) {
        // @todo Synchronize command pool
        vkFreeCommandBuffers(device, pUploadRequest->srcCmdPool, 1, &pUploadRequest->srcCmdBuffer);
    }

    delete pUploadRequest;
}