/*
    Copyright (C) 2022 George Cave.

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

#ifndef FOE_OPENXR_VK_RENDER_GRAPH_JOBS_SWAPCHAIN_HPP
#define FOE_OPENXR_VK_RENDER_GRAPH_JOBS_SWAPCHAIN_HPP

#include <foe/graphics/vk/render_graph.hpp>
#include <foe/openxr/vk/export.h>
#include <openxr/openxr.h>
#include <vulkan/vulkan.h>

#include <string_view>

struct RenderGraphXrSwapchain {
    RenderGraphResourceStructureType sType;
    void *pNext;
    XrSwapchain swapchain;
};

FOE_OPENXR_VK_EXPORT auto importXrSwapchain(foeGfxVkRenderGraph renderGraph,
                                            std::string_view jobName,
                                            VkFence fence,
                                            std::string_view resourceName,
                                            XrSwapchain swapchain,
                                            VkImage image,
                                            VkImageView view,
                                            VkFormat format,
                                            VkExtent2D extent) -> RenderGraphResource;

#endif // FOE_OPENXR_VK_RENDER_GRAPH_JOBS_SWAPCHAIN_HPP