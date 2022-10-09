// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RENDER_GRAPH_JOB_PRESENT_IMAGE_HPP
#define FOE_GRAPHICS_RENDER_GRAPH_JOB_PRESENT_IMAGE_HPP

#include <foe/graphics/export.h>
#include <foe/graphics/vk/render_graph.hpp>
#include <foe/result.h>
#include <vulkan/vulkan.h>

FOE_GFX_EXPORT foeResultSet
foeGfxVkImportSwapchainImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                      char const *pJobName,
                                      VkFence fence,
                                      char const *pResourceName,
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
FOE_GFX_EXPORT foeResultSet
foeGfxVkPresentSwapchainImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                       char const *pJobName,
                                       VkFence fence,
                                       foeGfxVkRenderGraphResource swapchainResource);

#endif // FOE_GRAPHICS_RENDER_GRAPH_JOB_PRESENT_IMAGE_HPP