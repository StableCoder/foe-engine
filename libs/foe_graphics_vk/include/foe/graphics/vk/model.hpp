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

#ifndef FOE_GRAPHICS_MODEL_HPP
#define FOE_GRAPHICS_MODEL_HPP

#include <foe/graphics/export.h>
#include <foe/graphics/resource_uploader.hpp>
#include <foe/graphics/upload_data.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

FOE_GFX_EXPORT VkResult allocateStagingBuffer(VmaAllocator allocator,
                                              VkDeviceSize size,
                                              VkBuffer *pStagingBuffer,
                                              VmaAllocation *pStagingAlloc);

FOE_GFX_EXPORT VkResult allocateModelBuffers(VmaAllocator allocator,
                                             VkDeviceSize vertexDataSize,
                                             VkDeviceSize indexDataSize,
                                             VkBuffer *pVertexBuffer,
                                             VmaAllocation *pVertexAlloc,
                                             VkBuffer *pIndexBuffer,
                                             VmaAllocation *pIndexAlloc,
                                             VkBuffer *pStagingBuffer,
                                             VmaAllocation *pStagingAlloc);

FOE_GFX_EXPORT VkResult mapModelBuffers(VmaAllocator allocator,
                                        VkDeviceSize vertexDataSize,
                                        VmaAllocation vertexAlloc,
                                        VmaAllocation indexAlloc,
                                        VmaAllocation stagingAlloc,
                                        void **ppVertexData,
                                        void **ppIndexData);

FOE_GFX_EXPORT void unmapModelBuffers(VmaAllocator allocator,
                                      VmaAllocation vertexAlloc,
                                      VmaAllocation indexAlloc,
                                      VmaAllocation stagingAlloc);

FOE_GFX_EXPORT VkResult recordModelUploadCommands(foeResourceUploader *pResourceUploader,
                                                  VkBuffer vertexBuffer,
                                                  VkDeviceSize vertexDataSize,
                                                  VkBuffer indexBuffer,
                                                  VkDeviceSize indexDataSize,
                                                  VkBuffer stagingBuffer,
                                                  foeUploadData *pUploadData);

#endif // FOE_GRAPHICS_MODEL_HPP