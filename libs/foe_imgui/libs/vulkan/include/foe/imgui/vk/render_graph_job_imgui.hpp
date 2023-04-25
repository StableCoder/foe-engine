// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_IMGUI_VK_RENDER_GRAPH_JOB_IMGUI_HPP
#define FOE_IMGUI_VK_RENDER_GRAPH_JOB_IMGUI_HPP

#include <foe/graphics/vk/render_graph.hpp>
#include <foe/imgui/vk/export.h>
#include <foe/result.h>

class foeImGuiRenderer;
class foeImGuiState;

FOE_IMGUI_VK_EXPORT
foeResultSet foeImGuiVkRenderUiJob(foeGfxVkRenderGraph renderGraph,
                                   char const *pJobName,
                                   VkFence fence,
                                   foeGfxVkRenderGraphResource renderTarget,
                                   uint32_t renderTargetUpstreamJobCount,
                                   foeGfxVkRenderGraphJob const *pRenderTargetUpstreamJobs,
                                   VkImageLayout finalLayout,
                                   foeImGuiRenderer *pImguiRenderer,
                                   foeImGuiState *pImguiState,
                                   uint32_t frameIndex,
                                   foeGfxVkRenderGraphJob *pRenderGraphJob);

#endif // FOE_IMGUI_VK_RENDER_GRAPH_JOB_IMGUI_HPP