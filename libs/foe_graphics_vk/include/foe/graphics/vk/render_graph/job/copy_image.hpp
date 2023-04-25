// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RENDER_GRAPH_JOB_COPY_IMAGE_HPP
#define FOE_GRAPHICS_RENDER_GRAPH_JOB_COPY_IMAGE_HPP

#include <foe/graphics/export.h>
#include <foe/graphics/vk/render_graph.hpp>
#include <foe/result.h>
#include <vulkan/vulkan.h>

FOE_GFX_EXPORT
foeResultSet foeGfxVkCopyImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                        char const *pJobName,
                                        VkFence fence,
                                        foeGfxVkRenderGraphResource srcImage,
                                        uint32_t srcImageUpstreamJobCount,
                                        foeGfxVkRenderGraphJob const *pSrcImageUpstreamJobs,
                                        VkImageLayout srcFinalLayout,
                                        foeGfxVkRenderGraphResource dstImage,
                                        uint32_t dstImageUpstreamJobCount,
                                        foeGfxVkRenderGraphJob const *pDstImageUpstreamJobs,
                                        VkImageLayout dstFinalLayout,
                                        foeGfxVkRenderGraphJob *pRenderGraphJob);

#endif // FOE_GRAPHICS_RENDER_GRAPH_JOB_COPY_IMAGE_HPP