// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/upload_context.h>

#include <foe/graphics/vk/session.h>

#include "result.h"
#include "session.hpp"
#include "upload_context.hpp"
#include "vk_result.h"

namespace {

void foeGfxVkDestroyUploadContext(foeGfxVkUploadContext *pUploadContext) {
    if (pUploadContext->dstCommandPool != VK_NULL_HANDLE)
        vkDestroyCommandPool(pUploadContext->device, pUploadContext->dstCommandPool, nullptr);

    if (pUploadContext->srcCommandPool != VK_NULL_HANDLE)
        vkDestroyCommandPool(pUploadContext->device, pUploadContext->srcCommandPool, nullptr);

    delete pUploadContext;
}

} // namespace

extern "C" foeResultSet foeGfxCreateUploadContext(foeGfxSession session,
                                                  foeGfxUploadContext *pUploadContext) {
    auto *pSession = session_from_handle(session);
    VkResult vkResult = VK_SUCCESS;

    auto transferQueue = foeGfxVkGetBestQueue(session, VK_QUEUE_TRANSFER_BIT);
    auto graphicsQueue = foeGfxVkGetBestQueue(session, VK_QUEUE_GRAPHICS_BIT);

    auto *pNewContext = new (std::nothrow) foeGfxVkUploadContext{
        .device = pSession->device,
        .allocator = pSession->allocator,
        .pSrcQueueFamily = (transferQueue == graphicsQueue)
                               ? VK_NULL_HANDLE
                               : &pSession->queueFamilies[transferQueue],
        .pDstQueueFamily = &pSession->queueFamilies[graphicsQueue],
    };
    if (pNewContext == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_OUT_OF_MEMORY);

    VkCommandPoolCreateInfo cmdPoolCI{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
    };

    if (pNewContext->pSrcQueueFamily != nullptr) {
        cmdPoolCI.queueFamilyIndex = transferQueue;

        vkResult = vkCreateCommandPool(pNewContext->device, &cmdPoolCI, nullptr,
                                       &pNewContext->srcCommandPool);
        if (vkResult != VK_SUCCESS)
            goto CREATE_FAILED;
    }

    if (pNewContext->pDstQueueFamily != nullptr) {
        cmdPoolCI.queueFamilyIndex = graphicsQueue;

        vkResult = vkCreateCommandPool(pNewContext->device, &cmdPoolCI, nullptr,
                                       &pNewContext->dstCommandPool);
        if (vkResult != VK_SUCCESS)
            goto CREATE_FAILED;
    }

CREATE_FAILED:
    if (vkResult != VK_SUCCESS) {
        foeGfxVkDestroyUploadContext(pNewContext);
    } else {
        *pUploadContext = upload_context_to_handle(pNewContext);
    }

    return vk_to_foeResult(vkResult);
}

extern "C" void foeGfxDestroyUploadContext(foeGfxUploadContext uploadContext) {
    auto *pUploadContext = upload_context_from_handle(uploadContext);

    foeGfxVkDestroyUploadContext(pUploadContext);
}