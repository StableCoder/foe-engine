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

FOE_OPENXR_VK_EXPORT
foeResultSet foeOpenXrVkImportSwapchainImageRenderJob(
    foeGfxVkRenderGraph renderGraph,
    char const *pJobName,
    VkFence fence,
    char const *pResourceName,
    VkSemaphore semaphore,
    XrSwapchain swapchain,
    VkImage image,
    VkImageView view,
    VkFormat format,
    VkExtent2D extent,
    VkImageLayout layout,
    foeGfxVkRenderGraphResource *pXrSwapchainResource,
    foeGfxVkRenderGraphJob *pRenderGraphJob);

#endif // FOE_XR_OPENXR_VK_RENDER_GRAPH_JOBS_SWAPCHAIN_HPP