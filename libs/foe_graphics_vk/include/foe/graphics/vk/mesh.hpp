// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_MESH_HPP
#define FOE_GRAPHICS_VK_MESH_HPP

#include <foe/error_code.h>
#include <foe/graphics/mesh.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_GFX_EXPORT foeResult foeGfxVkCreateMesh(foeGfxSession session,
                                            uint64_t vertexDataSize,
                                            uint64_t indexDataSize,
                                            uint32_t numIndices,
                                            VkIndexType indexType,
                                            uint64_t boneDataOffset,
                                            bool *pHostVisible,
                                            foeGfxMesh *pMesh);

FOE_GFX_EXPORT void foeGfxVkBindMesh(foeGfxMesh mesh,
                                     VkCommandBuffer commandBuffer,
                                     bool bindBoneData);

#ifdef __cplusplus
}
#endif

// TEMP
#include <tuple>
#include <vk_mem_alloc.h>

FOE_GFX_EXPORT std::tuple<VkBuffer, VmaAllocation> foeGfxVkGetMeshVertexData(foeGfxMesh mesh);
FOE_GFX_EXPORT std::tuple<VkBuffer, VmaAllocation> foeGfxVkGetMeshIndexData(foeGfxMesh mesh);

#endif // FOE_GRAPHICS_VK_MESH_HPP