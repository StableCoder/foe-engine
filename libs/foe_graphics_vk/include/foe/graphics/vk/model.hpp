// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

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