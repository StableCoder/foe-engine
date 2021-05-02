/*
    Copyright (C) 2021 George Cave.

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

#ifndef FOE_GRAPHICS_VK_YAML_FRAGMENT_DESCRIPTOR_HPP
#define FOE_GRAPHICS_VK_YAML_FRAGMENT_DESCRIPTOR_HPP

#include <foe/graphics/vk/fragment_descriptor.hpp>
#include <foe/graphics/vk/yaml/export.h>
#include <yaml-cpp/yaml.h>

#include <string>

FOE_GFX_VK_YAML_EXPORT
bool yaml_write_gfx_fragment_descriptor(std::string const &nodeName,
                                        foeGfxVkFragmentDescriptor const *pFragmentDescriptor,
                                        YAML::Node &node);

FOE_GFX_VK_YAML_EXPORT bool yaml_read_gfx_fragment_descriptor(
    std::string const &nodeName,
    YAML::Node const &node,
    bool &hasRasterizationSCI,
    VkPipelineRasterizationStateCreateInfo &rasterizationSCI,
    bool &hasDepthStencilSCI,
    VkPipelineDepthStencilStateCreateInfo &depthStencilSCI,
    bool &hasColourBlendSCI,
    VkPipelineColorBlendStateCreateInfo &colourBlendSCI,
    std::vector<VkPipelineColorBlendAttachmentState> &colourBlendAttachments);

#endif // FOE_GRAPHICS_VK_YAML_FRAGMENT_DESCRIPTOR_HPP