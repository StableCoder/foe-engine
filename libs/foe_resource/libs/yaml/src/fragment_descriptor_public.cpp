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

#include <foe/resource/imex/fragment_descriptor.hpp>

#include <foe/log.hpp>
#include <foe/yaml/exception.hpp>

#include "fragment_descriptor.hpp"

#include <fstream>

bool import_fragment_descriptor_definition(
    std::string_view fragmentDescriptorName,
    std::string &fragmentShader,
    VkPipelineRasterizationStateCreateInfo &rasterizationSCI,
    VkPipelineDepthStencilStateCreateInfo &depthStencilSCI,
    std::vector<VkPipelineColorBlendAttachmentState> &colourBlendAttachments,
    VkPipelineColorBlendStateCreateInfo &colourBlendSCI) {
    // Open the YAML file
    YAML::Node config;
    try {
        config = YAML::LoadFile(std::string{fragmentDescriptorName} + ".yml");
    } catch (YAML::ParserException &e) {
        FOE_LOG(General, Fatal, "Failed to load config file: {}", e.what());
    }

    try {
        yaml_read_fragment_descriptor_definition("", config, fragmentShader, rasterizationSCI,
                                                 depthStencilSCI, colourBlendAttachments,
                                                 colourBlendSCI);
    } catch (foeYamlException const &e) {
        FOE_LOG(General, Error, "Failed to import foeFragmentDescriptor definition: {}", e.what());
        return false;
    }

    return true;
}

bool export_fragment_descriptor_definition(foeFragmentDescriptor const *pFragmentDescriptor) {
    YAML::Node definition;

    try {
        yaml_write_fragment_descriptor_definition("", pFragmentDescriptor, definition);
    } catch (foeYamlException const &e) {
        FOE_LOG(General, Error, "Failed to export foeFragmentDescriptor definition: {}", e.what());
    }

    YAML::Emitter emitter;
    emitter << definition;

    std::ofstream outFile(std::string{pFragmentDescriptor->getName()} + ".yml", std::ofstream::out);
    if (outFile.is_open()) {
        outFile << emitter.c_str();
        outFile.close();
    } else {
        FOE_LOG(General, Error,
                "Failed to export foeFragmentDescriptor: Failed to open output file {}.yml",
                pFragmentDescriptor->getName());
        return false;
    }

    return true;
}