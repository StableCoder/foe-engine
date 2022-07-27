// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_YAML_VERTEX_DESCRIPTOR_HPP
#define FOE_GRAPHICS_VK_YAML_VERTEX_DESCRIPTOR_HPP

#include <foe/graphics/vk/vertex_descriptor.hpp>
#include <foe/graphics/vk/yaml/export.h>
#include <yaml-cpp/yaml.h>

#include <string>

FOE_GFX_VK_YAML_EXPORT
bool yaml_write_gfx_vertex_descriptor(std::string const &nodeName,
                                      foeGfxVertexDescriptor const *pVertexDescriptor,
                                      YAML::Node &node);

FOE_GFX_VK_YAML_EXPORT bool yaml_read_gfx_vertex_descriptor(
    std::string const &nodeName,
    YAML::Node const &node,
    VkPipelineVertexInputStateCreateInfo &vertexInputSCI,
    uint32_t &inputBindingCount,
    VkVertexInputBindingDescription *&inputBindings,
    uint32_t &inputAttributeCount,
    VkVertexInputAttributeDescription *&inputAttributes,
    VkPipelineInputAssemblyStateCreateInfo &inputAssemblySCI,
    VkPipelineTessellationStateCreateInfo &tessellationSCI);

#endif // FOE_GRAPHICS_VK_YAML_VERTEX_DESCRIPTOR_HPP