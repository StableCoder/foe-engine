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

#include <foe/graphics/upload_request.hpp>

#include <foe/graphics/upload_context.hpp>
#include <foe/graphics/vk/queue_family.hpp>
#include <vk_error_code.hpp>

#include "session.hpp"
#include "upload_context.hpp"
#include "upload_request.hpp"

void foeGfxDestroyUploadRequest(foeGfxUploadContext uploadContext,
                                foeGfxUploadRequest uploadRequest) {
    auto *pUploadContext = upload_context_from_handle(uploadContext);
    auto *pUploadRequest = upload_request_from_handle(uploadRequest);

    foeGfxVkDestroyUploadRequest(pUploadContext->device, pUploadRequest);
}

VkResult foeGfxVkCreateUploadData(VkDevice device,
                                  VkCommandPool srcCommandPool,
                                  VkCommandPool dstCommandPool,
                                  foeGfxUploadRequest *pUploadRequest) {
    VkResult res;
    auto *pNewRequest = new foeGfxVkUploadRequest;
    *pNewRequest = {
        .srcCmdPool = srcCommandPool,
        .dstCmdPool = dstCommandPool,
    };

    VkCommandBufferAllocateInfo bufferAI{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = pNewRequest->dstCmdPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkFenceCreateInfo fenceCI{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };

    // Destination
    res = vkAllocateCommandBuffers(device, &bufferAI, &pNewRequest->dstCmdBuffer);
    if (res != VK_NULL_HANDLE) {
        goto CREATE_FAILED;
    }

    res = vkCreateFence(device, &fenceCI, nullptr, &pNewRequest->dstFence);
    if (res != VK_SUCCESS) {
        goto CREATE_FAILED;
    }

    // Source
    if (pNewRequest->srcCmdPool != VK_NULL_HANDLE) {
        bufferAI.commandPool = pNewRequest->srcCmdPool;

        res = vkAllocateCommandBuffers(device, &bufferAI, &pNewRequest->srcCmdBuffer);
        if (res != VK_NULL_HANDLE) {
            goto CREATE_FAILED;
        }

        res = vkCreateFence(device, &fenceCI, nullptr, &pNewRequest->srcFence);
        if (res != VK_SUCCESS) {
            goto CREATE_FAILED;
        }

        { // Only need semaphore to synchronize if going across queues
            VkSemaphoreCreateInfo semaphoreCI{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            };

            res = vkCreateSemaphore(device, &semaphoreCI, nullptr, &pNewRequest->copyComplete);
            if (res != VK_SUCCESS) {
                goto CREATE_FAILED;
            }
        }
    }

CREATE_FAILED:
    if (res == VK_SUCCESS) {
        *pUploadRequest = upload_request_to_handle(pNewRequest);
    } else {
        foeGfxVkDestroyUploadRequest(device, pNewRequest);
    }

    return res;
}

std::error_code foeSubmitUploadDataCommands(foeGfxUploadContext uploadContext,
                                            foeGfxUploadRequest uploadRequest) {
    auto *pUploadContext = upload_context_from_handle(uploadContext);
    auto *pUploadRequest = upload_request_from_handle(uploadRequest);

    VkResult res;

    if (pUploadRequest->srcCmdBuffer != VK_NULL_HANDLE) { // Source command submission
        VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &pUploadRequest->srcCmdBuffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &pUploadRequest->copyComplete,
        };

        auto queue = foeGfxGetQueue(pUploadContext->srcQueueFamily);
        res = vkQueueSubmit(queue, 1, &submitInfo, pUploadRequest->srcFence);
        foeGfxReleaseQueue(pUploadContext->srcQueueFamily, queue);
        if (res != VK_SUCCESS) {
            return res;
        }
        pUploadRequest->srcSubmitted = true;
    }

    // Destination command submission
    VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = (pUploadRequest->copyComplete != VK_NULL_HANDLE) ? 1U : 0U,
        .pWaitSemaphores = &pUploadRequest->copyComplete,
        .pWaitDstStageMask = &waitStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &pUploadRequest->dstCmdBuffer,
    };

    auto queue = foeGfxGetQueue(pUploadContext->dstQueueFamily);
    res = vkQueueSubmit(queue, 1, &submitInfo, pUploadRequest->dstFence);
    foeGfxReleaseQueue(pUploadContext->dstQueueFamily, queue);
    if (res == VK_SUCCESS) {
        pUploadRequest->dstSubmitted = true;
    }

    // If the dst failed to submit but the src one *did*, then we need to wait for the source one to
    // complete or error-out before leaving.
    if (res != VK_SUCCESS && pUploadRequest->srcFence != VK_NULL_HANDLE) {
        while (vkGetFenceStatus(pUploadContext->device, pUploadRequest->srcFence) == VK_NOT_READY)
            ;
    }

    return res;
}

namespace {

UploadRequestStatus vkResultToRequestStatus(VkResult vkResult) {
    switch (vkResult) {
    case VK_SUCCESS:
        return FOE_GFX_UPLOAD_REQUEST_STATUS_COMPLETE;
    case VK_NOT_READY:
        return FOE_GFX_UPLOAD_REQUEST_STATUS_INCOMPLETE;
    default:
        return FOE_GFX_UPLOAD_REQUEST_STATUS_DEVICE_LOST;
    }
}

} // namespace

UploadRequestStatus foeGfxGetUploadRequestStatus(foeGfxUploadRequest uploadRequest) {
    auto *pUploadRequest = upload_request_from_handle(uploadRequest);

    // If the destination commands were submitted
    if (pUploadRequest->dstSubmitted) {
        return vkResultToRequestStatus(
            vkGetFenceStatus(pUploadRequest->device, pUploadRequest->dstFence));
    }

    // Otherwise, check the source one, presumably the dst ones failed to submit
    if (pUploadRequest->srcSubmitted) {
        return vkResultToRequestStatus(
            vkGetFenceStatus(pUploadRequest->device, pUploadRequest->srcFence));
    }

    return FOE_GFX_UPLOAD_REQUEST_STATUS_COMPLETE;
}