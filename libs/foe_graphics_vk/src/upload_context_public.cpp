/*
    Copyright (C) 2020-2022 George Cave.

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

#include <foe/graphics/upload_context.h>

#include <foe/graphics/vk/session.hpp>

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

extern "C" foeResult foeGfxCreateUploadContext(foeGfxSession session,
                                               foeGfxUploadContext *pUploadContext) {
    auto *pSession = session_from_handle(session);
    VkResult vkResult = VK_SUCCESS;
    auto *pNewContext = new foeGfxVkUploadContext;

    auto transferQueue = foeGfxVkGetBestQueue(session, VK_QUEUE_TRANSFER_BIT);
    auto graphicsQueue = foeGfxVkGetBestQueue(session, VK_QUEUE_GRAPHICS_BIT);

    *pNewContext = {
        .device = pSession->device,
        .allocator = pSession->allocator,
        .srcQueueFamily = (transferQueue == graphicsQueue)
                              ? VK_NULL_HANDLE
                              : &pSession->pQueueFamilies[transferQueue],
        .dstQueueFamily = &pSession->pQueueFamilies[graphicsQueue],
    };

    VkCommandPoolCreateInfo cmdPoolCI{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
    };

    if (pNewContext->srcQueueFamily != nullptr) {
        cmdPoolCI.queueFamilyIndex = transferQueue;

        vkResult = vkCreateCommandPool(pNewContext->device, &cmdPoolCI, nullptr,
                                       &pNewContext->srcCommandPool);
        if (vkResult != VK_SUCCESS)
            goto CREATE_FAILED;
    }

    if (pNewContext->dstQueueFamily != nullptr) {
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