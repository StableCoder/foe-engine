// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MESH_H
#define MESH_H

#include <foe/graphics/vk/mesh.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

struct foeGfxVkMesh {
    VkBuffer vertexBuffer;
    VmaAllocation vertexAlloc;
    VkBuffer indexBuffer;
    VmaAllocation indexAlloc;

    uint32_t numIndices;
    VkIndexType indexType;

    VkDeviceSize boneDataOffset;
};

FOE_DEFINE_HANDLE_CASTS(mesh, foeGfxVkMesh, foeGfxMesh)

#ifdef __cplusplus
}
#endif

#endif // MESH_H