// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_MODEL_H
#define FOE_GRAPHICS_MODEL_H

#include <foe/graphics/export.h>
#include <foe/graphics/upload_buffer.h>
#include <foe/graphics/upload_context.h>
#include <foe/graphics/upload_request.h>
#include <foe/result.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_GFX_EXPORT
foeResultSet foeGfxVkMapModelBuffers(VmaAllocator allocator,
                                     VkDeviceSize vertexDataSize,
                                     VmaAllocation vertexAlloc,
                                     VmaAllocation indexAlloc,
                                     foeGfxUploadContext uploadContext,
                                     foeGfxUploadBuffer uploadBuffer,
                                     void **ppVertexData,
                                     void **ppIndexData);

FOE_GFX_EXPORT
void foeGfxVkUnmapModelBuffers(VmaAllocator allocator,
                               VmaAllocation vertexAlloc,
                               VmaAllocation indexAlloc,
                               foeGfxUploadContext uploadContext,
                               foeGfxUploadBuffer uploadBuffer);

FOE_GFX_EXPORT
foeResultSet foeGfxVkRecordModelUploadCommands(foeGfxUploadContext uploadContext,
                                               VkBuffer vertexBuffer,
                                               VkDeviceSize vertexDataSize,
                                               VkBuffer indexBuffer,
                                               VkDeviceSize indexDataSize,
                                               foeGfxUploadBuffer uploadBuffer,
                                               foeGfxUploadRequest *pUploadRequest);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_MODEL_H