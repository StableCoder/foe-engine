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
#include <foe/graphics/session.hpp>
#include <vulkan/vulkan.h>

struct foeResourceUploader;

struct foeUploadData {
    VkCommandBuffer srcCmdBuffer;
    VkCommandPool srcCmdPool;
    VkFence srcFence;
    bool srcSubmitted;

    VkCommandBuffer dstCmdBuffer;
    VkCommandPool dstCmdPool;
    VkFence dstFence;
    bool dstSubmitted;

    VkSemaphore copyComplete;

    FOE_GFX_EXPORT void destroy(VkDevice device);
};

FOE_GFX_EXPORT VkResult foeCreateUploadData(VkDevice device,
                                            VkCommandPool srcCommandPool,
                                            VkCommandPool dstCommandPool,
                                            foeUploadData *pUploadData);

FOE_GFX_EXPORT VkResult foeSubmitUploadDataCommands(foeResourceUploader *pResourceUploader,
                                                    foeUploadData *pUploadData);

/**
 * @brief Returns the status of an upload request
 * @return VK_SUCCESS if the request hasn't been submitted, or if it has and has finished. An
 * appropriate value otherwise.
 */
FOE_GFX_EXPORT VkResult foeGfxGetUploadRequestStatus(VkDevice device,
                                                     foeUploadData const *pUploadData);

#endif // UPLOAD_DATA_HPP