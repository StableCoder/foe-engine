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

#ifndef FOE_GRAPHICS_RESOURCE_UPLOADER_HPP
#define FOE_GRAPHICS_RESOURCE_UPLOADER_HPP

#include <foe/graphics/device_environment.hpp>
#include <foe/graphics/export.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <mutex>

struct foeResourceUploader {
    VkDevice device;
    VmaAllocator allocator;

    foeQueueFamily *srcQueueFamily;
    VkCommandPool srcCommandPool;

    foeQueueFamily *dstQueueFamily;
    VkCommandPool dstCommandPool;
};

FOE_GFX_EXPORT VkResult foeGfxCreateResourceUploader(foeVkDeviceEnvironment *pGfxEnvironment,
                                                     foeResourceUploader *pResourceUploader);

FOE_GFX_EXPORT void foeGfxDestroyResourceUploader(foeResourceUploader *pResourceUploader);

#endif // FOE_GRAPHICS_RESOURCE_UPLOADER_HPP