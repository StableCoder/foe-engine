// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_RENDER_TARGET_H
#define FOE_GRAPHICS_VK_RENDER_TARGET_H

#include <foe/error_code.h>
#include <foe/graphics/delayed_caller.h>
#include <foe/graphics/render_target.h>
#include <foe/graphics/session.h>
#include <foe/graphics/vk/render_pass_pool.hpp>
#include <vulkan/vulkan.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct foeGfxVkRenderTargetSpec {
    VkFormat format;
    VkImageUsageFlags usage;
    // Number of images to rotate through
    uint32_t count;
};

FOE_GFX_EXPORT foeResultSet
foeGfxVkCreateRenderTarget(foeGfxSession session,
                           foeGfxDelayedCaller delayedCaller,
                           foeGfxVkRenderTargetSpec const *pSpecifications,
                           uint32_t count,
                           VkSampleCountFlags samples,
                           foeGfxRenderTarget *pRenderTarget);

FOE_GFX_EXPORT VkSampleCountFlags foeGfxVkGetRenderTargetSamples(foeGfxRenderTarget renderTarget);

FOE_GFX_EXPORT VkImage foeGfxVkGetRenderTargetImage(foeGfxRenderTarget renderTarget,
                                                    uint32_t index);

FOE_GFX_EXPORT VkImageView foeGfxVkGetRenderTargetImageView(foeGfxRenderTarget renderTarget,
                                                            uint32_t index);

FOE_GFX_EXPORT VkFramebuffer foeGfxVkGetRenderTargetFramebuffer(foeGfxRenderTarget renderTarget);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_VK_RENDER_TARGET_H