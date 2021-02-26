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

#include <foe/resource/yaml/material.hpp>

#include <foe/log.hpp>
#include <foe/yaml/exception.hpp>

#include "material.hpp"

#include <fstream>

bool import_yaml_material_definition(
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
    std::vector<VkPipelineColorBlendAttachmentState> &colourBlendAttachments) {
    // Open the YAML file
    YAML::Node rootNode;
    try {
        rootNode = YAML::LoadFile(std::string{materialName} + ".yml");
    } catch (YAML::ParserException &e) {
        FOE_LOG(General, Fatal, "Failed to load config file: {}", e.what());
    }

    try {
        return yaml_read_material_definition("", rootNode, fragmentShaderName, fragDescriptorName,
                                             image, hasRasterizationSCI, rasterizationSCI,
                                             hasDepthStencilSCI, depthStencilSCI, hasColourBlendSCI,
                                             colourBlendSCI, colourBlendAttachments);
    } catch (foeYamlException const &e) {
        FOE_LOG(General, Error, "Failed to import foeFragmentDescriptor definition: {}", e.what());
        return false;
    }
}

bool export_yaml_material_definition(foeMaterial const *pMaterial) {
    YAML::Node definition;

    try {
        yaml_write_material_definition("", pMaterial, definition);
    } catch (foeYamlException const &e) {
        FOE_LOG(General, Error, "Failed to export foeMaterial definition: {}", e.what());
    }

    YAML::Emitter emitter;
    emitter << definition;

    std::ofstream outFile(std::string{pMaterial->getName()} + ".yml", std::ofstream::out);
    if (outFile.is_open()) {
        outFile << emitter.c_str();
        outFile.close();
    } else {
        FOE_LOG(General, Error, "Failed to export foeMaterial: Failed to open output file {}.yml",
                pMaterial->getName());
        return false;
    }

    return true;
}