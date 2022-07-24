// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RENDER_GRAPH_JOB_BLIT_IMAGE_HPP
#define FOE_GRAPHICS_RENDER_GRAPH_JOB_BLIT_IMAGE_HPP

#include <foe/graphics/export.h>
#include <foe/graphics/vk/render_graph.hpp>
#include <foe/result.h>
#include <vulkan/vulkan.h>

#include <string_view>

struct BlitJobUsedResources {
    foeGfxVkRenderGraphResource srcImage;
    foeGfxVkRenderGraphResource dstImage;
};

FOE_GFX_EXPORT foeResultSet foeGfxVkBlitImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                                       std::string_view name,
                                                       VkFence fence,
                                                       foeGfxVkRenderGraphResource srcImage,
                                                       VkImageLayout srcFinalLayout,
                                                       foeGfxVkRenderGraphResource dstImage,
                                                       VkImageLayout dstFinalLayout,
                                                       BlitJobUsedResources *pResourcesOut);

#endif // FOE_GRAPHICS_RENDER_GRAPH_JOB_BLIT_IMAGE_HPP