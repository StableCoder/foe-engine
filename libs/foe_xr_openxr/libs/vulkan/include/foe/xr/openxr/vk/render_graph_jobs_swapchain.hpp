// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_XR_OPENXR_VK_RENDER_GRAPH_JOBS_SWAPCHAIN_HPP
#define FOE_XR_OPENXR_VK_RENDER_GRAPH_JOBS_SWAPCHAIN_HPP

#include <foe/graphics/vk/render_graph.hpp>
#include <foe/result.h>
#include <foe/xr/openxr/vk/export.h>
#include <openxr/openxr.h>
#include <vulkan/vulkan.h>

#include <string_view>

FOE_OPENXR_VK_EXPORT foeResultSet
foeOpenXrVkImportSwapchainImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                         std::string_view name,
                                         VkFence fence,
                                         std::string_view resourceName,
                                         XrSwapchain swapchain,
                                         VkImage image,
                                         VkImageView view,
                                         VkFormat format,
                                         VkExtent2D extent,
                                         VkImageLayout layout,
                                         foeGfxVkRenderGraphResource *pResourcesOut);

#endif // FOE_XR_OPENXR_VK_RENDER_GRAPH_JOBS_SWAPCHAIN_HPP