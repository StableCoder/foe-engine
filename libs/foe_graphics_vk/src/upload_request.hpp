// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef UPLOAD_REQUEST_HPP
#define UPLOAD_REQUEST_HPP

#include <foe/graphics/upload_request.h>
#include <vulkan/vulkan.h>

struct foeGfxVkUploadRequest {
    VkDevice device;

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

VkResult foeGfxVkCreateUploadRequest(VkDevice device,
                                     VkCommandPool srcCommandPool,
                                     VkCommandPool dstCommandPool,
                                     foeGfxVkUploadRequest **pUploadRequest);

void foeGfxVkDestroyUploadRequest(VkDevice device, foeGfxVkUploadRequest *pUploadRequest);

#endif // UPLOAD_REQUEST_HPP