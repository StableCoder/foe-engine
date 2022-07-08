// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/model.hpp>

#include <foe/graphics/vk/queue_family.hpp>

#include "result.h"
#include "upload_buffer.hpp"
#include "upload_context.hpp"
#include "upload_request.hpp"
#include "vk_result.h"

#include <array>

foeResult mapModelBuffers(VmaAllocator allocator,
                          VkDeviceSize vertexDataSize,
                          VmaAllocation vertexAlloc,
                          VmaAllocation indexAlloc,
                          foeGfxUploadContext uploadContext,
                          foeGfxUploadBuffer uploadBuffer,
                          void **ppVertexData,
                          void **ppIndexData) {
    if (uploadBuffer != FOE_NULL_HANDLE) {
        foeResult result = foeGfxMapUploadBuffer(uploadContext, uploadBuffer, ppVertexData);
        if (result.value != FOE_SUCCESS) {
            return result;
        }
        *ppIndexData = static_cast<uint8_t *>(*ppVertexData) + vertexDataSize;
    } else {
        VkResult vkResult;
        vkResult = vmaMapMemory(allocator, vertexAlloc, ppVertexData);
        if (vkResult != VK_SUCCESS) {
            return vk_to_foeResult(vkResult);
        }

        vkResult = vmaMapMemory(allocator, indexAlloc, ppIndexData);
        if (vkResult != VK_SUCCESS) {
            return vk_to_foeResult(vkResult);
        }
    }

    return to_foeResult(FOE_GRAPHICS_VK_SUCCESS);
}

void unmapModelBuffers(VmaAllocator allocator,
                       VmaAllocation vertexAlloc,
                       VmaAllocation indexAlloc,
                       foeGfxUploadContext uploadContext,
                       foeGfxUploadBuffer uploadBuffer) {
    if (uploadBuffer != FOE_NULL_HANDLE) {
        foeGfxUnmapUploadBuffer(uploadContext, uploadBuffer);
    } else {
        vmaUnmapMemory(allocator, indexAlloc);
        vmaUnmapMemory(allocator, vertexAlloc);
    }
}

foeResult recordModelUploadCommands(foeGfxUploadContext uploadContext,
                                    VkBuffer vertexBuffer,
                                    VkDeviceSize vertexDataSize,
                                    VkBuffer indexBuffer,
                                    VkDeviceSize indexDataSize,
                                    foeGfxUploadBuffer uploadBuffer,
                                    foeGfxUploadRequest *pUploadRequest) {
    auto *pUploadContext = upload_context_from_handle(uploadContext);

    VkResult vkResult;
    foeGfxVkUploadRequest *uploadRequest{nullptr};

    if (uploadBuffer != FOE_NULL_HANDLE) {
        // Need both queues for a tranfer
        vkResult = foeGfxVkCreateUploadData(pUploadContext->device, pUploadContext->srcCommandPool,
                                            pUploadContext->dstCommandPool, &uploadRequest);
    } else {
        vkResult = foeGfxVkCreateUploadData(pUploadContext->device, VK_NULL_HANDLE,
                                            pUploadContext->dstCommandPool, &uploadRequest);
    }
    if (vkResult != VK_SUCCESS) {
        goto RECORDING_FAILED;
    }

    { // Begin Recording
        VkCommandBufferBeginInfo cmdBufBI{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };

        if (uploadRequest->srcCmdBuffer != VK_NULL_HANDLE) {
            vkResult = vkBeginCommandBuffer(uploadRequest->srcCmdBuffer, &cmdBufBI);
            if (vkResult != VK_SUCCESS) {
                goto RECORDING_FAILED;
            }
        }

        vkResult = vkBeginCommandBuffer(uploadRequest->dstCmdBuffer, &cmdBufBI);
        if (vkResult != VK_SUCCESS) {
            goto RECORDING_FAILED;
        }
    }

    { // Recording
        if (uploadBuffer != FOE_NULL_HANDLE) {
            // Set memory barriers
            std::array<VkBufferMemoryBarrier, 2> transitionBarriers{
                VkBufferMemoryBarrier{
                    .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                    .srcAccessMask = 0,
                    .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .buffer = vertexBuffer,
                    .size = vertexDataSize,
                },
                VkBufferMemoryBarrier{
                    .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                    .srcAccessMask = 0,
                    .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .buffer = indexBuffer,
                    .size = indexDataSize,
                },
            };

            // If there's no src buffer, use the dst buffer
            auto commandBuffer = (uploadRequest->srcCmdBuffer != VK_NULL_HANDLE)
                                     ? uploadRequest->srcCmdBuffer
                                     : uploadRequest->dstCmdBuffer;

            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
                                 static_cast<uint32_t>(transitionBarriers.size()),
                                 transitionBarriers.data(), 0, nullptr);

            // Transfer data
            VkBufferCopy copyRegion{};
            auto *pGfxUploadBuffer = upload_buffer_from_handle(uploadBuffer);

            copyRegion.size = vertexDataSize;
            vkCmdCopyBuffer(commandBuffer, pGfxUploadBuffer->buffer, vertexBuffer, 1, &copyRegion);

            copyRegion.srcOffset = vertexDataSize;
            copyRegion.size = indexDataSize;
            vkCmdCopyBuffer(commandBuffer, pGfxUploadBuffer->buffer, indexBuffer, 1, &copyRegion);
        }

        // Transition to final states for use
        auto srcQueueFamily = (uploadRequest->srcCmdBuffer != VK_NULL_HANDLE)
                                  ? pUploadContext->srcQueueFamily->family
                                  : VK_QUEUE_FAMILY_IGNORED;

        auto dstQueueFamily = (uploadRequest->srcCmdBuffer != VK_NULL_HANDLE)
                                  ? pUploadContext->dstQueueFamily->family
                                  : VK_QUEUE_FAMILY_IGNORED;

        std::array<VkBufferMemoryBarrier, 2> transitionBarriers{
            VkBufferMemoryBarrier{
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
                .srcQueueFamilyIndex = srcQueueFamily,
                .dstQueueFamilyIndex = dstQueueFamily,
                .buffer = vertexBuffer,
                .size = vertexDataSize,
            },
            VkBufferMemoryBarrier{
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_INDEX_READ_BIT,
                .srcQueueFamilyIndex = srcQueueFamily,
                .dstQueueFamilyIndex = dstQueueFamily,
                .buffer = indexBuffer,
                .size = indexDataSize,
            },
        };

        if (uploadRequest->srcCmdBuffer) {
            vkCmdPipelineBarrier(uploadRequest->srcCmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
                                 static_cast<uint32_t>(transitionBarriers.size()),
                                 transitionBarriers.data(), 0, nullptr);
        }

        vkCmdPipelineBarrier(uploadRequest->dstCmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
                             static_cast<uint32_t>(transitionBarriers.size()),
                             transitionBarriers.data(), 0, nullptr);
    }

    { // End Recording
        if (uploadRequest->srcCmdBuffer != VK_NULL_HANDLE) {
            vkResult = vkEndCommandBuffer(uploadRequest->srcCmdBuffer);
            if (vkResult != VK_SUCCESS) {
                goto RECORDING_FAILED;
            }
        }

        vkResult = vkEndCommandBuffer(uploadRequest->dstCmdBuffer);
        if (vkResult != VK_SUCCESS) {
            goto RECORDING_FAILED;
        }
    }

RECORDING_FAILED:
    if (vkResult == VK_SUCCESS) {
        *pUploadRequest = upload_request_to_handle(uploadRequest);
    } else if (uploadRequest != FOE_NULL_HANDLE) {
        foeGfxVkDestroyUploadRequest(pUploadContext->device, uploadRequest);
    }

    return vk_to_foeResult(vkResult);
}