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

#ifndef FOE_GRAPHICS_VK_RENDER_TARGET_H
#define FOE_GRAPHICS_VK_RENDER_TARGET_H

#include <foe/error_code.h>
#include <foe/graphics/delayed_destructor.hpp>
#include <foe/graphics/render_target.h>
#include <foe/graphics/session.hpp>
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

FOE_GFX_EXPORT foeResult foeGfxVkCreateRenderTarget(foeGfxSession session,
                                                    foeGfxDelayedDestructor delayedDestructor,
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