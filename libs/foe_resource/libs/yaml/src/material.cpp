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

#include "material.hpp"

#include <foe/graphics/yaml/fragment_descriptor.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

#include "image.hpp"
#include "shader.hpp"

bool yaml_write_material_declaration(std::string const &nodeName,
                                     foeMaterial const *pMaterial,
                                     YAML::Node &node) {
    try {
        if (nodeName.empty()) {
            node = std::string{pMaterial->getName()};
        } else {
            node[nodeName] = std::string{pMaterial->getName()};
        }
    } catch (...) {
        throw foeYamlException(nodeName + " - Failed to serialize 'foeMaterial' declaration");
    }

    return true;
}

bool yaml_write_material_definition(std::string const &nodeName,
                                    foeMaterial const *pMaterial,
                                    YAML::Node &node) {
    YAML::Node writeNode;

    try {
        writeNode["index_id"] = pMaterial->getID();
        writeNode["editor_name"] = std::string{pMaterial->getName()};

        { // Resources Node
            YAML::Node resNode;

            // Fragment Shader
            if (auto *pFragShader = pMaterial->getFragmentShader(); pFragShader != nullptr) {
                yaml_write_shader_declaration("fragment_shader", pFragShader, resNode);
            }

            // Image
            if (auto *pImage = pMaterial->getImage(); pImage != nullptr) {
                yaml_write_image_declaration("image", pImage, resNode);
            }

            writeNode["resources"] = resNode;
        }

        // Gfx Data Node
        yaml_write_gfx_fragment_descriptor("graphics_data", pMaterial->getGfxFragmentDescriptor(),
                                           writeNode);
    } catch (...) {
        throw foeYamlException(nodeName + " - Failed to serialize 'foeMaterial' definition");
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }

    return true;
}

bool yaml_read_material_definition(
    std::string const &nodeName,
    YAML::Node const &node,
    foeResourceID &fragmentShader,
    std::string &fragmentDescriptor,
    foeResourceID &image,
    bool &hasRasterizationSCI,
    VkPipelineRasterizationStateCreateInfo &rasterizationSCI,
    bool &hasDepthStencilSCI,
    VkPipelineDepthStencilStateCreateInfo &depthStencilSCI,
    bool &hasColourBlendSCI,
    VkPipelineColorBlendStateCreateInfo &colourBlendSCI,
    std::vector<VkPipelineColorBlendAttachmentState> &colourBlendAttachments) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    try {
        // Resources
        if (auto resNode = subNode["resources"]; resNode) {
            yaml_read_optional("fragment_shader", resNode, fragmentShader);
            yaml_read_optional("fragment_descriptor", resNode, fragmentDescriptor);
            yaml_read_optional("image", resNode, image);
        }

        // Graphics Data
        yaml_read_gfx_fragment_descriptor(
            "graphics_data", subNode, hasRasterizationSCI, rasterizationSCI, hasDepthStencilSCI,
            depthStencilSCI, hasColourBlendSCI, colourBlendSCI, colourBlendAttachments);
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return true;
}