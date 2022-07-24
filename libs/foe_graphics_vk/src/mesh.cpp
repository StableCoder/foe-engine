// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/mesh.hpp>

#include "mesh.hpp"
#include "session.hpp"
#include "vk_result.h"

namespace {

void foeGfxVkDestroyMesh(foeGfxSession session, foeGfxVkMesh *pMesh) {
    auto *pSession = session_from_handle(session);

    if (pMesh->indexBuffer != VK_NULL_HANDLE)
        vmaDestroyBuffer(pSession->allocator, pMesh->indexBuffer, pMesh->indexAlloc);

    if (pMesh->vertexBuffer != VK_NULL_HANDLE)
        vmaDestroyBuffer(pSession->allocator, pMesh->vertexBuffer, pMesh->vertexAlloc);

    delete pMesh;
}

} // namespace

extern "C" foeResultSet foeGfxVkCreateMesh(foeGfxSession session,
                                           uint64_t vertexDataSize,
                                           uint64_t indexDataSize,
                                           uint32_t numIndices,
                                           VkIndexType indexType,
                                           uint64_t boneDataOffset,
                                           bool *pHostVisible,
                                           foeGfxMesh *pMesh) {
    auto *pSession = session_from_handle(session);
    VkResult vkResult = VK_SUCCESS;

    bool bothHostVisible = true;
    auto *pNewMesh = new foeGfxVkMesh;
    *pNewMesh = foeGfxVkMesh{
        .numIndices = numIndices,
        .indexType = indexType,
        .boneDataOffset = boneDataOffset,
    };

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
        vkResult = vmaCreateBuffer(pSession->allocator, &bufferCI, &allocCI,
                                   &pNewMesh->vertexBuffer, &pNewMesh->vertexAlloc, &allocInfo);
        if (vkResult != VK_SUCCESS) {
            goto CREATE_FAILED;
        }

        VkMemoryPropertyFlags memFlags;
        vmaGetMemoryTypeProperties(pSession->allocator, allocInfo.memoryType, &memFlags);
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
        vkResult = vmaCreateBuffer(pSession->allocator, &bufferCI, &allocCI, &pNewMesh->indexBuffer,
                                   &pNewMesh->indexAlloc, &allocInfo);
        if (vkResult != VK_SUCCESS) {
            goto CREATE_FAILED;
        }

        VkMemoryPropertyFlags memFlags;
        vmaGetMemoryTypeProperties(pSession->allocator, allocInfo.memoryType, &memFlags);
        if ((memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0) {
            bothHostVisible = false;
        }
    }

CREATE_FAILED:
    if (vkResult != VK_SUCCESS) {
        foeGfxVkDestroyMesh(session, pNewMesh);
    } else {
        *pMesh = mesh_to_handle(pNewMesh);
        *pHostVisible = bothHostVisible;
    }

    return vk_to_foeResult(vkResult);
}

extern "C" uint32_t foeGfxGetMeshIndices(foeGfxMesh mesh) {
    auto *pMesh = mesh_from_handle(mesh);

    return pMesh->numIndices;
}

extern "C" void foeGfxVkBindMesh(foeGfxMesh mesh,
                                 VkCommandBuffer commandBuffer,
                                 bool bindBoneData) {
    auto *pMesh = mesh_from_handle(mesh);

    std::array<VkDeviceSize, 2> offsets{0, pMesh->boneDataOffset};
    std::array<VkBuffer, 2> vertexBuffers{pMesh->vertexBuffer, pMesh->vertexBuffer};

    vkCmdBindVertexBuffers(commandBuffer, 0, (bindBoneData) ? 2 : 1, vertexBuffers.data(),
                           offsets.data());

    vkCmdBindIndexBuffer(commandBuffer, pMesh->indexBuffer, 0, pMesh->indexType);
}

extern "C" void foeGfxDestroyMesh(foeGfxSession session, foeGfxMesh mesh) {
    auto *pMesh = mesh_from_handle(mesh);

    foeGfxVkDestroyMesh(session, pMesh);
}

std::tuple<VkBuffer, VmaAllocation> foeGfxVkGetMeshVertexData(foeGfxMesh mesh) {
    auto *pMesh = mesh_from_handle(mesh);

    return std::make_tuple(pMesh->vertexBuffer, pMesh->vertexAlloc);
}

std::tuple<VkBuffer, VmaAllocation> foeGfxVkGetMeshIndexData(foeGfxMesh mesh) {
    auto *pMesh = mesh_from_handle(mesh);

    return std::make_tuple(pMesh->indexBuffer, pMesh->indexAlloc);
}