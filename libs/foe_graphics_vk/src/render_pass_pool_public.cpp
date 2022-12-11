// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/render_pass_pool.h>

#include "render_pass_pool.hpp"

extern "C" VkRenderPass foeGfxVkGetRenderPass(foeGfxVkRenderPassPool renderPassPool,
                                              uint32_t attachmentCount,
                                              VkAttachmentDescription const *pAttachments) {
    foeGfxVkRenderPassPoolImpl *pRenderPassPool = render_pass_pool_from_handle(renderPassPool);

    return pRenderPassPool->renderPass(attachmentCount, pAttachments);
}

extern "C" VkRenderPass foeGfxVkGetCompatibleRenderPass(foeGfxVkRenderPassPool renderPassPool,
                                                        uint32_t attachmentCount,
                                                        VkFormat const *pFormats,
                                                        VkSampleCountFlags const *pSampleFlags) {
    foeGfxVkRenderPassPoolImpl *pRenderPassPool = render_pass_pool_from_handle(renderPassPool);

    return pRenderPassPool->renderPass(attachmentCount, pFormats, pSampleFlags);
}