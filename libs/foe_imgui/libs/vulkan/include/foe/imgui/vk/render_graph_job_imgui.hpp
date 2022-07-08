// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_IMGUI_VK_RENDER_GRAPH_JOB_IMGUI_HPP
#define FOE_IMGUI_VK_RENDER_GRAPH_JOB_IMGUI_HPP

#include <foe/error_code.h>
#include <foe/graphics/vk/render_graph.hpp>
#include <foe/imgui/vk/export.h>

class foeImGuiRenderer;
class foeImGuiState;

FOE_IMGUI_VK_EXPORT foeResult foeImGuiVkRenderUiJob(foeGfxVkRenderGraph renderGraph,
                                                    std::string_view name,
                                                    VkFence fence,
                                                    foeGfxVkRenderGraphResource renderTarget,
                                                    VkImageLayout finalLayout,
                                                    foeImGuiRenderer *pImguiRenderer,
                                                    foeImGuiState *pImguiState,
                                                    uint32_t frameIndex,
                                                    foeGfxVkRenderGraphResource *pResourcesOut);

#endif // FOE_IMGUI_VK_RENDER_GRAPH_JOB_IMGUI_HPP