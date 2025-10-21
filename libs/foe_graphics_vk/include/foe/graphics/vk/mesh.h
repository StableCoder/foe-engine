// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_MESH_H
#define FOE_GRAPHICS_VK_MESH_H

#include <foe/external/vk_mem_alloc.h>
#include <foe/graphics/mesh.h>
#include <foe/result.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_GFX_EXPORT
foeResultSet foeGfxVkCreateMesh(foeGfxSession session,
                                uint64_t vertexDataSize,
                                uint64_t indexDataSize,
                                uint32_t numIndices,
                                VkIndexType indexType,
                                uint64_t boneDataOffset,
                                bool *pHostVisible,
                                foeGfxMesh *pMesh);

FOE_GFX_EXPORT
void foeGfxVkBindMesh(foeGfxMesh mesh, VkCommandBuffer commandBuffer, bool bindBoneData);

FOE_GFX_EXPORT
void foeGfxVkGetMeshVertexData(foeGfxMesh mesh, VkBuffer *pBuffer, VmaAllocation *pAllocation);
FOE_GFX_EXPORT
void foeGfxVkGetMeshIndexData(foeGfxMesh mesh, VkBuffer *pBuffer, VmaAllocation *pAllocation);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_VK_MESH_H