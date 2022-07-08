// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MESH_HPP
#define MESH_HPP

#include <foe/graphics/vk/mesh.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

struct foeGfxVkMesh {
    VkBuffer vertexBuffer{VK_NULL_HANDLE};
    VmaAllocation vertexAlloc{VK_NULL_HANDLE};
    VkBuffer indexBuffer{VK_NULL_HANDLE};
    VmaAllocation indexAlloc{VK_NULL_HANDLE};

    uint32_t numIndices{0};
    VkIndexType indexType{};

    VkDeviceSize boneDataOffset{0};
};

FOE_DEFINE_HANDLE_CASTS(mesh, foeGfxVkMesh, foeGfxMesh)

#endif // MESH_HPP