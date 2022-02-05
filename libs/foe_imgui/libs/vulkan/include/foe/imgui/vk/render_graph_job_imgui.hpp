/*
    Copyright (C) 2021 George Cave.

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

#ifndef FOE_IMGUI_VK_RENDER_GRAPH_JOB_IMGUI_HPP
#define FOE_IMGUI_VK_RENDER_GRAPH_JOB_IMGUI_HPP

#include <foe/graphics/vk/render_graph.hpp>
#include <foe/imgui/vk/export.h>

class foeImGuiRenderer;
class foeImGuiState;

FOE_IMGUI_VK_EXPORT auto foeImGuiVkRenderUiJob(foeGfxVkRenderGraph renderGraph,
                                               std::string_view name,
                                               VkFence fence,
                                               RenderGraphResource renderTarget,
                                               VkImageLayout initialLayout,
                                               VkImageLayout finalLayout,
                                               foeImGuiRenderer *pImguiRenderer,
                                               foeImGuiState *pImguiState,
                                               uint32_t frameIndex) -> RenderGraphResource;

#endif // FOE_IMGUI_VK_RENDER_GRAPH_JOB_IMGUI_HPP