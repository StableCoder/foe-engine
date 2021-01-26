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

#include "upload_request.hpp"

VkResult foeGfxVkCreateUploadData(VkDevice device,
                                  VkCommandPool srcCommandPool,
                                  VkCommandPool dstCommandPool,
                                  foeGfxVkUploadRequest **pUploadData) {
    VkResult res;
    auto uploadData = new foeGfxVkUploadRequest;
    *uploadData = {
        .srcCmdPool = srcCommandPool,
        .dstCmdPool = dstCommandPool,
    };

    VkCommandBufferAllocateInfo bufferAI{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = uploadData->dstCmdPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkFenceCreateInfo fenceCI{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };

    // Destination
    res = vkAllocateCommandBuffers(device, &bufferAI, &uploadData->dstCmdBuffer);
    if (res != VK_NULL_HANDLE) {
        goto CREATE_FAILED;
    }

    res = vkCreateFence(device, &fenceCI, nullptr, &uploadData->dstFence);
    if (res != VK_SUCCESS) {
        goto CREATE_FAILED;
    }

    // Source
    if (uploadData->srcCmdPool != VK_NULL_HANDLE) {
        bufferAI.commandPool = uploadData->srcCmdPool;

        res = vkAllocateCommandBuffers(device, &bufferAI, &uploadData->srcCmdBuffer);
        if (res != VK_NULL_HANDLE) {
            goto CREATE_FAILED;
        }

        res = vkCreateFence(device, &fenceCI, nullptr, &uploadData->srcFence);
        if (res != VK_SUCCESS) {
            goto CREATE_FAILED;
        }

        { // Only need semaphore to synchronize if going across queues
            VkSemaphoreCreateInfo semaphoreCI{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            };

            res = vkCreateSemaphore(device, &semaphoreCI, nullptr, &uploadData->copyComplete);
            if (res != VK_SUCCESS) {
                goto CREATE_FAILED;
            }
        }
    }

CREATE_FAILED:
    if (res == VK_SUCCESS) {
        *pUploadData = uploadData;
    } else {
        foeGfxVkDestroyUploadRequest(device, uploadData);
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