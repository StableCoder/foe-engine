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

#ifndef FOE_RESOURCE_YAML_MATERIAL_HPP
#define FOE_RESOURCE_YAML_MATERIAL_HPP

#include <foe/resource/material.hpp>
#include <foe/resource/yaml/export.h>
#include <vulkan/vulkan.h>

#include <string>
#include <string_view>
#include <vector>

FOE_RES_YAML_EXPORT bool import_yaml_material_definition(
    std::string_view materialName,
    std::string &fragmentShaderName,
    std::string &fragDescriptorName,
    std::string &image,
    bool &hasRasterizationSCI,
    VkPipelineRasterizationStateCreateInfo &rasterizationSCI,
    bool &hasDepthStencilSCI,
    VkPipelineDepthStencilStateCreateInfo &depthStencilSCI,
    bool &hasColourBlendSCI,
    VkPipelineColorBlendStateCreateInfo &colourBlendSCI,
    std::vector<VkPipelineColorBlendAttachmentState> &colourBlendAttachments);

FOE_RES_YAML_EXPORT bool export_yaml_material_definition(foeMaterial const *pMaterial);

#endif // FOE_RESOURCE_YAML_MATERIAL_HPP