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

#ifndef UPLOAD_CONTEXT_HPP
#define UPLOAD_CONTEXT_HPP

#include <foe/graphics/export.h>
#include <foe/graphics/session.h>
#include <foe/graphics/upload_context.h>
#include <foe/graphics/vk/queue_family.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <mutex>

struct foeGfxVkUploadContext {
    VkDevice device;
    VmaAllocator allocator;

    foeGfxVkQueueFamily *srcQueueFamily;
    VkCommandPool srcCommandPool;

    foeGfxVkQueueFamily *dstQueueFamily;
    VkCommandPool dstCommandPool;
};

FOE_DEFINE_HANDLE_CASTS(upload_context, foeGfxVkUploadContext, foeGfxUploadContext)

#endif // UPLOAD_CONTEXT_HPP