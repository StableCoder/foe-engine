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

#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <foe/resource/material.hpp>
#include <yaml-cpp/yaml.h>

#include <string>

bool yaml_write_material_declaration(std::string const &nodeName,
                                     foeMaterial const *pMaterial,
                                     YAML::Node &node);

bool yaml_write_material_definition(std::string const &nodeName,
                                    foeMaterial const *pMaterial,
                                    YAML::Node &node);

bool yaml_read_material_definition(
    std::string const &nodeName,
    YAML::Node const &node,
    std::string &fragmentShader,
    std::string &fragmentDescriptor,
    std::string &image,
    bool &hasRasterizationSCI,
    VkPipelineRasterizationStateCreateInfo &rasterizationSCI,
    bool &hasDepthStencilSCI,
    VkPipelineDepthStencilStateCreateInfo &depthStencilSCI,
    bool &hasColourBlendSCI,
    VkPipelineColorBlendStateCreateInfo &colourBlendSCI,
    std::vector<VkPipelineColorBlendAttachmentState> &colourBlendAttachments);

#endif // MATERIAL_HPP