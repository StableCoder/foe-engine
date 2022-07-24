// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/upload_request.h>

#include <foe/graphics/upload_context.h>
#include <foe/graphics/vk/queue_family.hpp>

#include "session.hpp"
#include "upload_context.hpp"
#include "upload_request.hpp"
#include "vk_result.h"

extern "C" void foeGfxDestroyUploadRequest(foeGfxUploadContext uploadContext,
                                           foeGfxUploadRequest uploadRequest) {
    auto *pUploadContext = upload_context_from_handle(uploadContext);
    auto *pUploadRequest = upload_request_from_handle(uploadRequest);

    foeGfxVkDestroyUploadRequest(pUploadContext->device, pUploadRequest);
}

extern "C" foeResultSet foeGfxVkCreateUploadData(VkDevice device,
                                                 VkCommandPool srcCommandPool,
                                                 VkCommandPool dstCommandPool,
                                                 foeGfxUploadRequest *pUploadRequest) {
    VkResult vkResult;
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
    vkResult = vkAllocateCommandBuffers(device, &bufferAI, &pNewRequest->dstCmdBuffer);
    if (vkResult != VK_SUCCESS) {
        goto CREATE_FAILED;
    }

    vkResult = vkCreateFence(device, &fenceCI, nullptr, &pNewRequest->dstFence);
    if (vkResult != VK_SUCCESS) {
        goto CREATE_FAILED;
    }

    // Source
    if (pNewRequest->srcCmdPool != VK_NULL_HANDLE) {
        bufferAI.commandPool = pNewRequest->srcCmdPool;

        vkResult = vkAllocateCommandBuffers(device, &bufferAI, &pNewRequest->srcCmdBuffer);
        if (vkResult != VK_SUCCESS) {
            goto CREATE_FAILED;
        }

        vkResult = vkCreateFence(device, &fenceCI, nullptr, &pNewRequest->srcFence);
        if (vkResult != VK_SUCCESS) {
            goto CREATE_FAILED;
        }

        { // Only need semaphore to synchronize if going across queues
            VkSemaphoreCreateInfo semaphoreCI{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            };

            vkResult = vkCreateSemaphore(device, &semaphoreCI, nullptr, &pNewRequest->copyComplete);
            if (vkResult != VK_SUCCESS) {
                goto CREATE_FAILED;
            }
        }
    }

CREATE_FAILED:
    if (vkResult == VK_SUCCESS) {
        *pUploadRequest = upload_request_to_handle(pNewRequest);
    } else {
        foeGfxVkDestroyUploadRequest(device, pNewRequest);
    }

    return vk_to_foeResult(vkResult);
}

extern "C" foeResultSet foeSubmitUploadDataCommands(foeGfxUploadContext uploadContext,
                                                    foeGfxUploadRequest uploadRequest) {
    auto *pUploadContext = upload_context_from_handle(uploadContext);
    auto *pUploadRequest = upload_request_from_handle(uploadRequest);

    VkResult vkResult;

    if (pUploadRequest->srcCmdBuffer != VK_NULL_HANDLE) { // Source command submission
        VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &pUploadRequest->srcCmdBuffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &pUploadRequest->copyComplete,
        };

        auto queue = foeGfxGetQueue(pUploadContext->srcQueueFamily);
        vkResult = vkQueueSubmit(queue, 1, &submitInfo, pUploadRequest->srcFence);
        foeGfxReleaseQueue(pUploadContext->srcQueueFamily, queue);
        if (vkResult != VK_SUCCESS) {
            return vk_to_foeResult(vkResult);
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
    vkResult = vkQueueSubmit(queue, 1, &submitInfo, pUploadRequest->dstFence);
    foeGfxReleaseQueue(pUploadContext->dstQueueFamily, queue);
    if (vkResult == VK_SUCCESS) {
        pUploadRequest->dstSubmitted = true;
    }

    // If the dst failed to submit but the src one *did*, then we need to wait for the source one to
    // complete or error-out before leaving.
    if (vkResult != VK_SUCCESS && pUploadRequest->srcFence != VK_NULL_HANDLE) {
        while (vkGetFenceStatus(pUploadContext->device, pUploadRequest->srcFence) == VK_NOT_READY)
            ;
    }

    return vk_to_foeResult(vkResult);
}

namespace {

foeGfxUploadRequestStatus vkResultToRequestStatus(VkResult vkResult) {
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

extern "C" foeGfxUploadRequestStatus foeGfxGetUploadRequestStatus(
    foeGfxUploadRequest uploadRequest) {
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