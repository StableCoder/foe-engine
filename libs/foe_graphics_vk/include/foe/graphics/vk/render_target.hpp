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

#ifndef FOE_GRAPHICS_VK_RENDER_TARGET_HPP
#define FOE_GRAPHICS_VK_RENDER_TARGET_HPP

#include <foe/error_code.h>
#include <foe/graphics/delayed_destructor.hpp>
#include <foe/graphics/render_target.hpp>
#include <foe/graphics/session.hpp>
#include <foe/graphics/vk/render_pass_pool.hpp>
#include <vulkan/vulkan.h>

#include <cstdint>

struct foeGfxVkRenderTargetSpec {
    VkFormat format;
    VkImageUsageFlags usage;
    // Number of images to rotate through
    uint32_t count;
};

FOE_GFX_EXPORT foeResult foeGfxVkCreateRenderTarget(foeGfxSession session,
                                                    foeGfxDelayedDestructor delayedDestructor,
                                                    foeGfxVkRenderTargetSpec const *pSpecifications,
                                                    uint32_t count,
                                                    VkSampleCountFlags samples,
                                                    foeGfxRenderTarget *pRenderTarget);

FOE_GFX_EXPORT auto foeGfxVkGetRenderTargetSamples(foeGfxRenderTarget renderTarget)
    -> VkSampleCountFlags;

FOE_GFX_EXPORT auto foeGfxVkGetRenderTargetImage(foeGfxRenderTarget renderTarget, uint32_t index)
    -> VkImage;

FOE_GFX_EXPORT auto foeGfxVkGetRenderTargetImageView(foeGfxRenderTarget renderTarget,
                                                     uint32_t index) -> VkImageView;

FOE_GFX_EXPORT auto foeGfxVkGetRenderTargetFramebuffer(foeGfxRenderTarget renderTarget)
    -> VkFramebuffer;

#endif // FOE_GRAPHICS_VK_RENDER_TARGET_HPP