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

#include "fragment_descriptor.hpp"

#include <foe/graphics/yaml/fragment_descriptor.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

#include "shader.hpp"

bool yaml_write_fragment_descriptor_declaration(std::string const &nodeName,
                                                foeFragmentDescriptor const *pFragmentDescriptor,
                                                YAML::Node &node) {
    try {
        if (nodeName.empty()) {
            node = std::string{pFragmentDescriptor->getName()};
        } else {
            node[nodeName] = std::string{pFragmentDescriptor->getName()};
        }
    } catch (...) {
        throw foeYamlException(nodeName +
                               " - Failed to serialize 'foeFragmentDescriptor' declaration");
    }

    return true;
}

bool yaml_write_fragment_descriptor_definition(std::string const &nodeName,
                                               foeFragmentDescriptor const *pFragmentDescriptor,
                                               YAML::Node &node) {
    YAML::Node writeNode;

    try {
        { // Resources Node
            YAML::Node resNode;

            // Fragment Shader
            if (auto *pFragShader = pFragmentDescriptor->getFragmentShader();
                pFragShader != nullptr) {
                yaml_write_shader_declaration("fragment_shader", pFragShader, resNode);
            }

            writeNode["resources"] = resNode;
        }

        // Gfx Data Node
        yaml_write_gfx_fragment_descriptor("graphics_data",
                                           pFragmentDescriptor->getFragmentDescriptor(), writeNode);
    } catch (...) {
        throw foeYamlException(nodeName +
                               " - Failed to serialize 'foeFragmentDescriptor' definition");
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }

    return true;
}

bool yaml_read_fragment_descriptor_definition(
    std::string const &nodeName,
    YAML::Node const &node,
    std::string &fragmentShader,
    VkPipelineRasterizationStateCreateInfo &rasterizationSCI,
    VkPipelineDepthStencilStateCreateInfo &depthStencilSCI,
    std::vector<VkPipelineColorBlendAttachmentState> &colourBlendAttachments,
    VkPipelineColorBlendStateCreateInfo &colourBlendSCI) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    try {
        {
            auto resNode = subNode["resources"];

            // Resources
            yaml_read_optional("fragment_shader", resNode, fragmentShader);
        }

        // Graphics Data
        yaml_read_gfx_fragment_descriptor("graphics_data", subNode, rasterizationSCI,
                                          depthStencilSCI, colourBlendAttachments, colourBlendSCI);
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return true;
}