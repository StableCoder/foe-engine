// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/upload_buffer.h>

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

extern "C" foeResult foeGfxCreateUploadBuffer(foeGfxUploadContext uploadContext,
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

extern "C" void foeGfxDestroyUploadBuffer(foeGfxUploadContext uploadContext,
                                          foeGfxUploadBuffer uploadBuffer) {
    auto *pUploadBuffer = upload_buffer_from_handle(uploadBuffer);

    foeGfxVkDestroyUploadBuffer(uploadContext, pUploadBuffer);
}

extern "C" foeResult foeGfxMapUploadBuffer(foeGfxUploadContext uploadContext,
                                           foeGfxUploadBuffer uploadBuffer,
                                           void **ppData) {
    auto *pUploadContext = upload_context_from_handle(uploadContext);
    auto *pUploadBuffer = upload_buffer_from_handle(uploadBuffer);

    return vk_to_foeResult(vmaMapMemory(pUploadContext->allocator, pUploadBuffer->alloc, ppData));
}

extern "C" void foeGfxUnmapUploadBuffer(foeGfxUploadContext uploadContext,
                                        foeGfxUploadBuffer uploadBuffer) {
    auto *pUploadContext = upload_context_from_handle(uploadContext);
    auto *pUploadBuffer = upload_buffer_from_handle(uploadBuffer);

    vmaUnmapMemory(pUploadContext->allocator, pUploadBuffer->alloc);
}