// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RENDER_TARGET_HPP
#define RENDER_TARGET_HPP

#include <foe/external/vk_mem_alloc.h>
#include <foe/graphics/delayed_caller.h>
#include <foe/graphics/render_target.h>
#include <foe/graphics/vk/render_target.h>
#include <vulkan/vulkan.h>

#include <vector>

struct foeGfxVkSession;

struct RenderTargetImageData {
    VmaAllocation alloc;
    VkImage image;
    VkImageView view;
    bool latest;
};

struct foeGfxVkRenderTarget {
    foeGfxVkSession const *const pSession;
    foeGfxDelayedCaller const delayedCaller;
    std::vector<foeGfxVkRenderTargetSpec> const imageSpecifications;
    VkSampleCountFlags const samples;
    VkRenderPass const compatibleRenderPass;

    VkExtent2D extent;

    std::vector<RenderTargetImageData> images;
    VkFramebuffer framebuffer;

    std::vector<uint8_t> indices;
};

FOE_DEFINE_HANDLE_CASTS(render_target, foeGfxVkRenderTarget, foeGfxRenderTarget)

#endif // RENDER_TARGET_HPP