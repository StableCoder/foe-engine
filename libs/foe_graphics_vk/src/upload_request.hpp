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

#include <foe/graphics/upload_request.hpp>
#include <vulkan/vulkan.h>

struct foeGfxVkUploadRequest {
    VkCommandBuffer srcCmdBuffer;
    VkCommandPool srcCmdPool;
    VkFence srcFence;
    bool srcSubmitted;

    VkCommandBuffer dstCmdBuffer;
    VkCommandPool dstCmdPool;
    VkFence dstFence;
    bool dstSubmitted;

    VkSemaphore copyComplete;
};

FOE_DEFINE_HANDLE_CASTS(upload_request, foeGfxVkUploadRequest, foeGfxUploadRequest)

VkResult foeCreateUploadData(VkDevice device,
                             VkCommandPool srcCommandPool,
                             VkCommandPool dstCommandPool,
                             foeGfxVkUploadRequest **pUploadData);

void foeGfxVkDestroyUploadRequest(VkDevice device, foeGfxVkUploadRequest *pUploadRequest);

#endif // UPLOAD_DATA_HPP