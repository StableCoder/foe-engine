/*
    Copyright (C) 2021 George Cave.

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