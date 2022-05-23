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

#ifndef FOE_GRAPHICS_VK_MESH_HPP
#define FOE_GRAPHICS_VK_MESH_HPP

#include <foe/error_code.h>
#include <foe/graphics/mesh.hpp>
#include <vulkan/vulkan.h>

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

// TEMP
#include <tuple>
#include <vk_mem_alloc.h>

FOE_GFX_EXPORT std::tuple<VkBuffer, VmaAllocation> foeGfxVkGetMeshVertexData(foeGfxMesh mesh);
FOE_GFX_EXPORT std::tuple<VkBuffer, VmaAllocation> foeGfxVkGetMeshIndexData(foeGfxMesh mesh);

#endif // FOE_GRAPHICS_VK_MESH_HPP