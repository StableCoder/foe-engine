/*
    Copyright (C) 2020-2022 George Cave.

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

#include <foe/error_code.h>
#include <foe/graphics/export.h>
#include <foe/graphics/upload_buffer.h>
#include <foe/graphics/upload_context.h>
#include <foe/graphics/upload_request.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

FOE_GFX_EXPORT foeResult mapModelBuffers(VmaAllocator allocator,
                                         VkDeviceSize vertexDataSize,
                                         VmaAllocation vertexAlloc,
                                         VmaAllocation indexAlloc,
                                         foeGfxUploadContext uploadContext,
                                         foeGfxUploadBuffer uploadBuffer,
                                         void **ppVertexData,
                                         void **ppIndexData);

FOE_GFX_EXPORT void unmapModelBuffers(VmaAllocator allocator,
                                      VmaAllocation vertexAlloc,
                                      VmaAllocation indexAlloc,
                                      foeGfxUploadContext uploadContext,
                                      foeGfxUploadBuffer uploadBuffer);

FOE_GFX_EXPORT foeResult recordModelUploadCommands(foeGfxUploadContext uploadContext,
                                                   VkBuffer vertexBuffer,
                                                   VkDeviceSize vertexDataSize,
                                                   VkBuffer indexBuffer,
                                                   VkDeviceSize indexDataSize,
                                                   foeGfxUploadBuffer uploadBuffer,
                                                   foeGfxUploadRequest *pUploadRequest);

#endif // FOE_GRAPHICS_MODEL_HPP