// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RENDER_GRAPH_JOB_IMPORT_IMAGE_HPP
#define FOE_GRAPHICS_RENDER_GRAPH_JOB_IMPORT_IMAGE_HPP

#include <foe/graphics/export.h>
#include <foe/graphics/vk/render_graph.hpp>
#include <foe/result.h>
#include <vulkan/vulkan.h>

#include <vector>

FOE_GFX_EXPORT foeResultSet
foeGfxVkImportImageRenderJob(foeGfxVkRenderGraph renderGraph,
                             char const *pJobName,
                             VkFence fence,
                             char const *pResourceName,
                             VkImage image,
                             VkImageView view,
                             VkFormat format,
                             VkExtent2D extent,
                             VkImageLayout layout,
                             bool isMutable,
                             std::vector<VkSemaphore> waitSemaphores,
                             foeGfxVkRenderGraphResource *pImportedImageResource,
                             foeGfxVkRenderGraphJob *pRenderGraphJob);

#endif // FOE_GRAPHICS_RENDER_GRAPH_JOB_IMPORT_IMAGE_HPP