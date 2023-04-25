// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RENDER_GRAPH_JOB_PRESENT_IMAGE_HPP
#define FOE_GRAPHICS_RENDER_GRAPH_JOB_PRESENT_IMAGE_HPP

#include <foe/graphics/export.h>
#include <foe/graphics/vk/render_graph.hpp>
#include <foe/result.h>
#include <vulkan/vulkan.h>

typedef struct foeGfxVkSwapchainPresentInfo {
    foeGfxVkRenderGraphResource swapchainResource;
    uint32_t upstreamJobCount;
    foeGfxVkRenderGraphJob *pUpstreamJobs;
} foeGfxVkSwapchainPresentInfo;

FOE_GFX_EXPORT
foeResultSet foeGfxVkImportSwapchainImageRenderJob(foeGfxVkRenderGraph renderGraph,
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
                                                   foeGfxVkRenderGraphResource *pSwapchainResource,
                                                   foeGfxVkRenderGraphJob *pRenderGraphJob);

FOE_GFX_EXPORT
foeResultSet foeGfxVkPresentSwapchainImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                                    char const *pJobName,
                                                    VkFence fence,
                                                    uint32_t presentInfoCount,
                                                    foeGfxVkSwapchainPresentInfo *pPresentInfos);

#endif // FOE_GRAPHICS_RENDER_GRAPH_JOB_PRESENT_IMAGE_HPP