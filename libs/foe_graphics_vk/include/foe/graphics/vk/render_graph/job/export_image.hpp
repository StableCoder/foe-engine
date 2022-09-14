// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RENDER_GRAPH_JOB_EXPORT_IMAGE_HPP
#define FOE_GRAPHICS_RENDER_GRAPH_JOB_EXPORT_IMAGE_HPP

#include <foe/graphics/export.h>
#include <foe/graphics/vk/render_graph.hpp>
#include <foe/result.h>
#include <vulkan/vulkan.h>

#include <string_view>
#include <vector>

FOE_GFX_EXPORT foeResultSet foeGfxVkExportImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                                         std::string_view name,
                                                         VkFence fence,
                                                         foeGfxVkRenderGraphResource resource,
                                                         VkImageLayout requiredLayout,
                                                         std::vector<VkSemaphore> signalSemaphores);

#endif // FOE_GRAPHICS_RENDER_GRAPH_JOB_EXPORT_IMAGE_HPP