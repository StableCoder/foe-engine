// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_RENDER_VIEW_POOL_H
#define FOE_GRAPHICS_VK_RENDER_VIEW_POOL_H

#include <foe/graphics/export.h>
#include <foe/graphics/render_view_pool.h>
#include <vulkan/vulkan.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_GFX_EXPORT
VkDescriptorSet foeGfxVkGetRenderViewDescriptorSet(foeGfxRenderViewPool renderViewPool,
                                                   foeGfxRenderView renderView);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_VK_RENDER_VIEW_POOL_H