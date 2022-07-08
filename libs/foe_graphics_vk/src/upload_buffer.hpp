// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef UPLOAD_BUFFER_HPP
#define UPLOAD_BUFFER_HPP

#include <foe/graphics/upload_buffer.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

struct foeGfxVkUploadBuffer {
    VkBuffer buffer;
    VmaAllocation alloc;
};

FOE_DEFINE_HANDLE_CASTS(upload_buffer, foeGfxVkUploadBuffer, foeGfxUploadBuffer)

#endif // UPLOAD_BUFFER_HPP