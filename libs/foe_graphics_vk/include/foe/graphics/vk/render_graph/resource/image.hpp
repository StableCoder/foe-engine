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