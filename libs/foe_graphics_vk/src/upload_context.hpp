// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef UPLOAD_CONTEXT_HPP
#define UPLOAD_CONTEXT_HPP

#include <foe/external/vk_mem_alloc.h>
#include <foe/graphics/export.h>
#include <foe/graphics/session.h>
#include <foe/graphics/upload_context.h>
#include <vulkan/vulkan.h>

#include "queue_family.hpp"

#include <mutex>

struct foeGfxVkUploadContext {
    VkDevice device;
    VmaAllocator allocator;

    QueueFamily *pSrcQueueFamily;
    VkCommandPool srcCommandPool;

    QueueFamily *pDstQueueFamily;
    VkCommandPool dstCommandPool;
};

FOE_DEFINE_HANDLE_CASTS(upload_context, foeGfxVkUploadContext, foeGfxUploadContext)

#endif // UPLOAD_CONTEXT_HPP