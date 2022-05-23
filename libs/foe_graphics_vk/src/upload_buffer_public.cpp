/*
    Copyright (C) 2021-2022 George Cave.

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

#include <foe/graphics/upload_buffer.hpp>

#include "upload_buffer.hpp"
#include "upload_context.hpp"
#include "vk_result.h"

namespace {

void foeGfxVkDestroyUploadBuffer(foeGfxUploadContext uploadContext,
                                 foeGfxVkUploadBuffer *pUploadBuffer) {
    auto *pUploadContext = upload_context_from_handle(uploadContext);

    if (pUploadBuffer->buffer != VK_NULL_HANDLE)
        vmaDestroyBuffer(pUploadContext->allocator, pUploadBuffer->buffer, pUploadBuffer->alloc);

    delete pUploadBuffer;
}

} // namespace

foeResult foeGfxCreateUploadBuffer(foeGfxUploadContext uploadContext,
                                   uint64_t size,
                                   foeGfxUploadBuffer *pUploadBuffer) {
    auto *pUploadContext = upload_context_from_handle(uploadContext);

    VkBuffer stagingBuffer{VK_NULL_HANDLE};
    VmaAllocation stagingAlloc{VK_NULL_HANDLE};

    VkBufferCreateInfo bufferCI{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    };

    VmaAllocationCreateInfo allocCI{
        .usage = VMA_MEMORY_USAGE_CPU_ONLY,
    };

    VkResult vkRes = vmaCreateBuffer(pUploadContext->allocator, &bufferCI, &allocCI, &stagingBuffer,
                                     &stagingAlloc, nullptr);
    if (vkRes != VK_SUCCESS)
        goto ALLOCATION_FAILED;

ALLOCATION_FAILED:
    if (vkRes != VK_SUCCESS) {
        // On fail, delete what we locally created
        if (stagingBuffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(pUploadContext->allocator, stagingBuffer, stagingAlloc);
        }
    } else {
        // On success, create and return the buffer data
        auto *pNew = new foeGfxVkUploadBuffer;
        *pNew = {
            .buffer = stagingBuffer,
            .alloc = stagingAlloc,
        };
        *pUploadBuffer = upload_buffer_to_handle(pNew);
    }

    return vk_to_foeResult(vkRes);
}

void foeGfxDestroyUploadBuffer(foeGfxUploadContext uploadContext, foeGfxUploadBuffer uploadBuffer) {
    auto *pUploadBuffer = upload_buffer_from_handle(uploadBuffer);

    foeGfxVkDestroyUploadBuffer(uploadContext, pUploadBuffer);
}

foeResult foeGfxMapUploadBuffer(foeGfxUploadContext uploadContext,
                                foeGfxUploadBuffer uploadBuffer,
                                void **ppData) {
    auto *pUploadContext = upload_context_from_handle(uploadContext);
    auto *pUploadBuffer = upload_buffer_from_handle(uploadBuffer);

    return vk_to_foeResult(vmaMapMemory(pUploadContext->allocator, pUploadBuffer->alloc, ppData));
}

void foeGfxUnmapUploadBuffer(foeGfxUploadContext uploadContext, foeGfxUploadBuffer uploadBuffer) {
    auto *pUploadContext = upload_context_from_handle(uploadContext);
    auto *pUploadBuffer = upload_buffer_from_handle(uploadBuffer);

    vmaUnmapMemory(pUploadContext->allocator, pUploadBuffer->alloc);
}