/*
    Copyright (C) 2021-2022 George Cave.

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

#ifndef RENDER_TARGET_HPP
#define RENDER_TARGET_HPP

#include <foe/graphics/delayed_caller.h>
#include <foe/graphics/render_target.h>
#include <foe/graphics/vk/render_target.h>
#include <vk_mem_alloc.h>
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