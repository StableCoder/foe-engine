// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "upload_request.hpp"

VkResult foeGfxVkCreateUploadRequest(VkDevice device,
                                     VkCommandPool srcCommandPool,
                                     VkCommandPool dstCommandPool,
                                     foeGfxVkUploadRequest **pUploadRequest) {
    VkResult res;
    auto uploadRequest = new foeGfxVkUploadRequest;
    *uploadRequest = {
        .device = device,
        .srcCmdPool = srcCommandPool,
        .dstCmdPool = dstCommandPool,
    };

    VkCommandBufferAllocateInfo bufferAI{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = uploadRequest->dstCmdPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkFenceCreateInfo fenceCI{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };

    // Destination
    res = vkAllocateCommandBuffers(device, &bufferAI, &uploadRequest->dstCmdBuffer);
    if (res != VK_SUCCESS) {
        goto CREATE_FAILED;
    }

    res = vkCreateFence(device, &fenceCI, nullptr, &uploadRequest->dstFence);
    if (res != VK_SUCCESS) {
        goto CREATE_FAILED;
    }

    // Source
    if (uploadRequest->srcCmdPool != VK_NULL_HANDLE) {
        bufferAI.commandPool = uploadRequest->srcCmdPool;

        res = vkAllocateCommandBuffers(device, &bufferAI, &uploadRequest->srcCmdBuffer);
        if (res != VK_SUCCESS) {
            goto CREATE_FAILED;
        }

        res = vkCreateFence(device, &fenceCI, nullptr, &uploadRequest->srcFence);
        if (res != VK_SUCCESS) {
            goto CREATE_FAILED;
        }

        { // Only need semaphore to synchronize if going across queues
            VkSemaphoreCreateInfo semaphoreCI{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            };

            res = vkCreateSemaphore(device, &semaphoreCI, nullptr, &uploadRequest->copyComplete);
            if (res != VK_SUCCESS) {
                goto CREATE_FAILED;
            }
        }
    }

CREATE_FAILED:
    if (res == VK_SUCCESS) {
        *pUploadRequest = uploadRequest;
    } else {
        foeGfxVkDestroyUploadRequest(device, uploadRequest);
    }

    return res;
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