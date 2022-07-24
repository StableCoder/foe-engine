// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RENDER_GRAPH_JOB_IMPORT_IMAGE_HPP
#define FOE_GRAPHICS_RENDER_GRAPH_JOB_IMPORT_IMAGE_HPP

#include <foe/error_code.h>
#include <foe/graphics/export.h>
#include <foe/graphics/vk/render_graph.hpp>
#include <vulkan/vulkan.h>

#include <string_view>

FOE_GFX_EXPORT foeResultSet foeGfxVkImportImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                                         std::string_view name,
                                                         VkFence fence,
                                                         std::string_view imageName,
                                                         VkImage image,
                                                         VkImageView view,
                                                         VkFormat format,
                                                         VkExtent2D extent,
                                                         VkImageLayout layout,
                                                         bool isMutable,
                                                         std::vector<VkSemaphore> waitSemaphores,
                                                         foeGfxVkRenderGraphResource *pResourceOut);

#endif // FOE_GRAPHICS_RENDER_GRAPH_JOB_IMPORT_IMAGE_HPP