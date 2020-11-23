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

#ifndef UPLOAD_DATA_HPP
#define UPLOAD_DATA_HPP

#include <foe/graphics/export.h>
#include <vulkan/vulkan.h>

struct foeResourceUploader;

struct foeUploadData {
    VkCommandBuffer srcCmdBuffer;
    VkCommandPool srcCmdPool;
    VkFence srcFence;

    VkCommandBuffer dstCmdBuffer;
    VkCommandPool dstCmdPool;
    VkFence dstFence;

    VkSemaphore copyComplete;

    FOE_GFX_EXPORT void destroy(VkDevice device);
};

FOE_GFX_EXPORT VkResult createUploadData(foeResourceUploader *pResourceUploader,
                                         foeUploadData *pUploadData);

FOE_GFX_EXPORT VkResult submitUploadDataCommands(foeResourceUploader *pResourceUploader,
                                                 foeUploadData *pUploadData);

#endif // UPLOAD_DATA_HPP