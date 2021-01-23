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

#include <foe/graphics/model.hpp>

#include <array>

VkResult allocateStagingBuffer(VmaAllocator allocator,
                               VkDeviceSize size,
                               VkBuffer *pStagingBuffer,
                               VmaAllocation *pStagingAlloc) {
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

    VkResult vkRes =
        vmaCreateBuffer(allocator, &bufferCI, &allocCI, &stagingBuffer, &stagingAlloc, nullptr);
    if (vkRes != VK_SUCCESS) {
        goto ALLOCATION_FAILED;
    }

ALLOCATION_FAILED:
    if (vkRes != VK_SUCCESS) {
        if (stagingBuffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(allocator, stagingBuffer, stagingAlloc);
        }
    } else {

        *pStagingBuffer = stagingBuffer;
        *pStagingAlloc = stagingAlloc;
    }

    return vkRes;
}

VkResult allocateModelBuffers(VmaAllocator allocator,
                              VkDeviceSize vertexDataSize,
                              VkDeviceSize indexDataSize,
                              VkBuffer *pVertexBuffer,
                              VmaAllocation *pVertexAlloc,
                              VkBuffer *pIndexBuffer,
                              VmaAllocation *pIndexAlloc,
                              VkBuffer *pStagingBuffer,
                              VmaAllocation *pStagingAlloc) {
    VkResult res;

    VkBuffer vertexBuffer{VK_NULL_HANDLE};
    VmaAllocation vertexAlloc{VK_NULL_HANDLE};
    VkBuffer indexBuffer{VK_NULL_HANDLE};
    VmaAllocation indexAlloc{VK_NULL_HANDLE};
    VkBuffer stagingBuffer{VK_NULL_HANDLE};
    VmaAllocation stagingAlloc{VK_NULL_HANDLE};
    bool bothHostVisible = true;

    { // Vertex Buffer
        VkBufferCreateInfo bufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = vertexDataSize,
            .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        };

        VmaAllocationCreateInfo allocCI{
            .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        };

        VmaAllocationInfo allocInfo;
        res = vmaCreateBuffer(allocator, &bufferCI, &allocCI, &vertexBuffer, &vertexAlloc,
                              &allocInfo);
        if (res != VK_SUCCESS) {
            goto ALLOCATION_FAILED;
        }

        VkMemoryPropertyFlags memFlags;
        vmaGetMemoryTypeProperties(allocator, allocInfo.memoryType, &memFlags);
        if ((memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0) {
            bothHostVisible = false;
        }
    }

    { // Index Buffer
        VkBufferCreateInfo bufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = indexDataSize,
            .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        };

        VmaAllocationCreateInfo allocCI{
            .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        };

        VmaAllocationInfo allocInfo;
        res =
            vmaCreateBuffer(allocator, &bufferCI, &allocCI, &indexBuffer, &indexAlloc, &allocInfo);
        if (res != VK_SUCCESS) {
            goto ALLOCATION_FAILED;
        }

        VkMemoryPropertyFlags memFlags;
        vmaGetMemoryTypeProperties(allocator, allocInfo.memoryType, &memFlags);
        if ((memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0) {
            bothHostVisible = false;
        }
    }

    if (!bothHostVisible) {
        VkBufferCreateInfo bufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = vertexDataSize + indexDataSize,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        };

        VmaAllocationCreateInfo allocCI{
            .usage = VMA_MEMORY_USAGE_CPU_ONLY,
        };

        res =
            vmaCreateBuffer(allocator, &bufferCI, &allocCI, &stagingBuffer, &stagingAlloc, nullptr);
        if (res != VK_SUCCESS) {
            goto ALLOCATION_FAILED;
        }
    }

ALLOCATION_FAILED:
    if (res != VK_SUCCESS) {
        if (stagingBuffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(allocator, stagingBuffer, stagingAlloc);
        }

        if (indexBuffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(allocator, indexBuffer, indexAlloc);
        }

        if (vertexBuffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(allocator, vertexBuffer, vertexAlloc);
        }
    } else {
        *pVertexBuffer = vertexBuffer;
        *pVertexAlloc = vertexAlloc;
        *pIndexBuffer = indexBuffer;
        *pIndexAlloc = indexAlloc;
        *pStagingBuffer = stagingBuffer;
        *pStagingAlloc = stagingAlloc;
    }

    return res;
}

VkResult mapModelBuffers(VmaAllocator allocator,
                         VkDeviceSize vertexDataSize,
                         VmaAllocation vertexAlloc,
                         VmaAllocation indexAlloc,
                         VmaAllocation stagingAlloc,
                         void **ppVertexData,
                         void **ppIndexData) {
    VkResult res;

    if (stagingAlloc != VK_NULL_HANDLE) {
        res = vmaMapMemory(allocator, stagingAlloc, ppVertexData);
        if (res != VK_SUCCESS) {
            return res;
        }
        *ppIndexData = static_cast<uint8_t *>(*ppVertexData) + vertexDataSize;
    } else {
        res = vmaMapMemory(allocator, vertexAlloc, ppVertexData);
        if (res != VK_SUCCESS) {
            return res;
        }

        res = vmaMapMemory(allocator, indexAlloc, ppIndexData);
        if (res != VK_SUCCESS) {
            return res;
        }
    }

    return res;
}

void unmapModelBuffers(VmaAllocator allocator,
                       VmaAllocation vertexAlloc,
                       VmaAllocation indexAlloc,
                       VmaAllocation stagingAlloc) {
    if (stagingAlloc != VK_NULL_HANDLE) {
        vmaUnmapMemory(allocator, stagingAlloc);
    } else {
        vmaUnmapMemory(allocator, indexAlloc);
        vmaUnmapMemory(allocator, vertexAlloc);
    }
}

#include <foe/graphics/upload_data.hpp>

VkResult recordModelUploadCommands(foeResourceUploader *pResourceUploader,
                                   VkBuffer vertexBuffer,
                                   VkDeviceSize vertexDataSize,
                                   VkBuffer indexBuffer,
                                   VkDeviceSize indexDataSize,
                                   VkBuffer stagingBuffer,
                                   foeUploadData *pUploadData) {
    VkResult res;
    foeUploadData uploadData;

    if (stagingBuffer) {
        // Need both queues for a tranfer
        res = foeCreateUploadData(pResourceUploader->device, pResourceUploader->srcCommandPool,
                                  pResourceUploader->dstCommandPool, &uploadData);
    } else {
        res = foeCreateUploadData(pResourceUploader->device, VK_NULL_HANDLE,
                                  pResourceUploader->dstCommandPool, &uploadData);
    }
    if (res != VK_SUCCESS) {
        goto RECORDING_FAILED;
    }

    { // Begin Recording
        VkCommandBufferBeginInfo cmdBufBI{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };

        if (uploadData.srcCmdBuffer != VK_NULL_HANDLE) {
            res = vkBeginCommandBuffer(uploadData.srcCmdBuffer, &cmdBufBI);
            if (res != VK_SUCCESS) {
                goto RECORDING_FAILED;
            }
        }

        res = vkBeginCommandBuffer(uploadData.dstCmdBuffer, &cmdBufBI);
        if (res != VK_SUCCESS) {
            goto RECORDING_FAILED;
        }
    }

    { // Recording
        if (stagingBuffer != VK_NULL_HANDLE) {
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
            auto commandBuffer = (uploadData.srcCmdBuffer != VK_NULL_HANDLE)
                                     ? uploadData.srcCmdBuffer
                                     : uploadData.dstCmdBuffer;

            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
                                 transitionBarriers.size(), transitionBarriers.data(), 0, nullptr);

            // Transfer data
            VkBufferCopy copyRegion{};

            copyRegion.size = vertexDataSize;
            vkCmdCopyBuffer(commandBuffer, stagingBuffer, vertexBuffer, 1, &copyRegion);

            copyRegion.srcOffset = vertexDataSize;
            copyRegion.size = indexDataSize;
            vkCmdCopyBuffer(commandBuffer, stagingBuffer, indexBuffer, 1, &copyRegion);
        }

        // Transition to final states for use
        auto srcQueueFamily = (uploadData.srcCmdBuffer != VK_NULL_HANDLE)
                                  ? pResourceUploader->srcQueueFamily->family
                                  : VK_QUEUE_FAMILY_IGNORED;

        auto dstQueueFamily = (uploadData.srcCmdBuffer != VK_NULL_HANDLE)
                                  ? pResourceUploader->dstQueueFamily->family
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

        if (uploadData.srcCmdBuffer) {
            vkCmdPipelineBarrier(uploadData.srcCmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
                                 transitionBarriers.size(), transitionBarriers.data(), 0, nullptr);
        }

        vkCmdPipelineBarrier(uploadData.dstCmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
                             transitionBarriers.size(), transitionBarriers.data(), 0, nullptr);
    }

    { // End Recording
        if (uploadData.srcCmdBuffer != VK_NULL_HANDLE) {
            res = vkEndCommandBuffer(uploadData.srcCmdBuffer);
            if (res != VK_SUCCESS) {
                goto RECORDING_FAILED;
            }
        }

        res = vkEndCommandBuffer(uploadData.dstCmdBuffer);
        if (res != VK_SUCCESS) {
            goto RECORDING_FAILED;
        }
    }

RECORDING_FAILED:
    if (res == VK_SUCCESS) {
        *pUploadData = uploadData;
    } else {
        uploadData.destroy(pResourceUploader->device);
    }

    return res;
}