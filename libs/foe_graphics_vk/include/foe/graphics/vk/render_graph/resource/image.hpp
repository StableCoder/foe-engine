// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RENDER_GRAPH_RESOURCES_IMAGE_HPP
#define FOE_GRAPHICS_RENDER_GRAPH_RESOURCES_IMAGE_HPP

#include <foe/graphics/vk/render_graph.hpp>

struct foeGfxVkGraphImageResource {
    foeGfxVkRenderGraphStructureType sType;
    void *pNext;
    std::string name;
    VkImage image;
    VkImageView view;
    VkFormat format;
    VkExtent2D extent;
    bool isMutable;
};

struct foeGfxVkGraphImageState {
    foeGfxVkRenderGraphStructureType sType;
    void *pNext;
    VkImageLayout layout;
};

FOE_GFX_EXPORT VkAccessFlags foeGfxVkDetermineAccessFlags(VkImageLayout imageLayout);

#endif // FOE_GRAPHICS_RENDER_GRAPH_RESOURCES_IMAGE_HPP