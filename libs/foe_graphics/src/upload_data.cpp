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

#include <foe/graphics/upload_data.hpp>

#include <foe/graphics/resource_uploader.hpp>

void foeUploadData::destroy(VkDevice device) {
    if (copyComplete != VK_NULL_HANDLE) {
        vkDestroySemaphore(device, copyComplete, nullptr);
    }

    if (dstFence != VK_NULL_HANDLE) {
        vkDestroyFence(device, dstFence, nullptr);
    }

    if (dstCmdBuffer != VK_NULL_HANDLE) {
        vkFreeCommandBuffers(device, dstCmdPool, 1, &dstCmdBuffer);
    }

    if (srcFence != VK_NULL_HANDLE) {
        vkDestroyFence(device, srcFence, nullptr);
    }

    if (srcCmdBuffer != VK_NULL_HANDLE) {
        vkFreeCommandBuffers(device, srcCmdPool, 1, &srcCmdBuffer);
    }
}

VkResult foeCreateUploadData(VkDevice device,
                             VkCommandPool srcCommandPool,
                             VkCommandPool dstCommandPool,
                             foeUploadData *pUploadData) {
    VkResult res;
    foeUploadData uploadData{
        .srcCmdPool = srcCommandPool,
        .dstCmdPool = dstCommandPool,
    };

    VkCommandBufferAllocateInfo bufferAI{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = uploadData.dstCmdPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkFenceCreateInfo fenceCI{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };

    // Destination
    res = vkAllocateCommandBuffers(device, &bufferAI, &uploadData.dstCmdBuffer);
    if (res != VK_NULL_HANDLE) {
        goto CREATE_FAILED;
    }

    res = vkCreateFence(device, &fenceCI, nullptr, &uploadData.dstFence);
    if (res != VK_SUCCESS) {
        goto CREATE_FAILED;
    }

    // Source
    if (uploadData.srcCmdPool != VK_NULL_HANDLE) {
        bufferAI.commandPool = uploadData.srcCmdPool;

        res = vkAllocateCommandBuffers(device, &bufferAI, &uploadData.srcCmdBuffer);
        if (res != VK_NULL_HANDLE) {
            goto CREATE_FAILED;
        }

        res = vkCreateFence(device, &fenceCI, nullptr, &uploadData.srcFence);
        if (res != VK_SUCCESS) {
            goto CREATE_FAILED;
        }

        { // Only need semaphore to synchronize if going across queues
            VkSemaphoreCreateInfo semaphoreCI{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            };

            res = vkCreateSemaphore(device, &semaphoreCI, nullptr, &uploadData.copyComplete);
            if (res != VK_SUCCESS) {
                goto CREATE_FAILED;
            }
        }
    }

CREATE_FAILED:
    if (res == VK_SUCCESS) {
        *pUploadData = uploadData;
    } else {
        uploadData.destroy(device);
    }

    return res;
}

VkResult foeSubmitUploadDataCommands(foeResourceUploader *pResourceUploader,
                                     foeUploadData *pUploadData) {
    VkResult res;

    if (pUploadData->srcCmdBuffer != VK_NULL_HANDLE) { // Source command submission
        VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &pUploadData->srcCmdBuffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &pUploadData->copyComplete,
        };

        auto queue = pResourceUploader->srcQueueFamily->queue[0];
        res = vkQueueSubmit(queue, 1, &submitInfo, pUploadData->srcFence);
        if (res != VK_SUCCESS) {
            return res;
        }
    }

    // Destination command submission
    VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = (pUploadData->copyComplete != VK_NULL_HANDLE) ? 1U : 0U,
        .pWaitSemaphores = &pUploadData->copyComplete,
        .pWaitDstStageMask = &waitStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &pUploadData->dstCmdBuffer,
    };

    auto queue = pResourceUploader->dstQueueFamily->queue[0];
    res = vkQueueSubmit(queue, 1, &submitInfo, pUploadData->dstFence);

    // If the dst failed to submit but the src one *did*, then we need to wait for the source one to
    // complete or error-out before leaving.
    if (res != VK_SUCCESS && pUploadData->srcFence != VK_NULL_HANDLE) {
        while (vkGetFenceStatus(pResourceUploader->device, pUploadData->srcFence) == VK_NOT_READY)
            ;
    }

    return res;
}