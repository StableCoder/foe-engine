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

#ifndef FOE_GRAPHICS_RENDER_GRAPH_JOB_PRESENT_IMAGE_HPP
#define FOE_GRAPHICS_RENDER_GRAPH_JOB_PRESENT_IMAGE_HPP

#include <foe/error_code.h>
#include <foe/graphics/export.h>
#include <foe/graphics/vk/render_graph.hpp>
#include <vulkan/vulkan.h>

#include <string_view>

FOE_GFX_EXPORT foeResult
foeGfxVkImportSwapchainImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                      std::string_view name,
                                      VkFence fence,
                                      std::string_view resourceName,
                                      VkSwapchainKHR swapchain,
                                      uint32_t index,
                                      VkImage image,
                                      VkImageView view,
                                      VkFormat format,
                                      VkExtent2D extent,
                                      VkImageLayout initialLayout,
                                      VkSemaphore waitSemaphore,
                                      foeGfxVkRenderGraphResource *pResourceOut);

/// Assumes the image is in VK_IMAGE_LAYOUT_PRESENT_SRC_KHR layout
FOE_GFX_EXPORT foeResult
foeGfxVkPresentSwapchainImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                       std::string_view name,
                                       VkFence fence,
                                       foeGfxVkRenderGraphResource swapchainResource);

#endif // FOE_GRAPHICS_RENDER_GRAPH_JOB_PRESENT_IMAGE_HPP