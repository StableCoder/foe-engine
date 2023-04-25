// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RENDER_GRAPH_JOB_SYNCHRONIZE_HPP
#define FOE_GRAPHICS_RENDER_GRAPH_JOB_SYNCHRONIZE_HPP

#include <foe/graphics/export.h>
#include <foe/graphics/vk/render_graph.hpp>
#include <foe/result.h>

FOE_GFX_EXPORT
foeResultSet foeGfxVkSynchronizeJob(foeGfxVkRenderGraph renderGraph,
                                    char const *pJobName,
                                    bool required,
                                    VkFence fence,
                                    uint32_t upstreamJobCount,
                                    foeGfxVkRenderGraphJob *pUpstreamJobs,
                                    foeGfxVkRenderGraphJob *pJob);

#endif // FOE_GRAPHICS_RENDER_GRAPH_JOB_SYNCHRONIZE_HPP