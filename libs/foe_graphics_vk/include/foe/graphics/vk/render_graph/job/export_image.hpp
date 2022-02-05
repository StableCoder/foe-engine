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

#ifndef FOE_GRAPHICS_RENDER_GRAPH_JOB_EXPORT_IMAGE_HPP
#define FOE_GRAPHICS_RENDER_GRAPH_JOB_EXPORT_IMAGE_HPP

#include <foe/graphics/export.h>
#include <foe/graphics/vk/render_graph.hpp>
#include <vulkan/vulkan.h>

#include <string_view>

FOE_GFX_EXPORT void foeGfxVkExportImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                                 std::string_view name,
                                                 VkFence fence,
                                                 RenderGraphResource resource,
                                                 VkImageLayout layout,
                                                 std::vector<VkSemaphore> signalSemaphores);

#endif // FOE_GRAPHICS_RENDER_GRAPH_JOB_EXPORT_IMAGE_HPP