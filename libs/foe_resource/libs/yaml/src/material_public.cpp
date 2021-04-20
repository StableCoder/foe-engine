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

#include <foe/ecs/yaml/id.hpp>
#include <foe/graphics/yaml/fragment_descriptor.hpp>
#include <foe/log.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

#include "material.hpp"

#include <fstream>

namespace {

bool yaml_read_material_definition_internal(std::string const &nodeName,
                                            YAML::Node const &node,
                                            foeIdGroupTranslator const *pTranslator,
                                            foeMaterialCreateInfo &createInfo) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    bool read{false};

    try {
        // Resources
        if (auto resNode = subNode["resources"]; resNode) {
            read |= yaml_read_id_optional("fragment_shader", resNode, pTranslator,
                                          foeIdTypeResource, createInfo.fragmentShader);
            read |= yaml_read_id_optional("image", resNode, pTranslator, foeIdTypeResource,
                                          createInfo.image);

            read |=
                yaml_read_optional("fragment_descriptor", resNode, createInfo.fragDescriptorName);
        }

        // Graphics Data
        read |= yaml_read_gfx_fragment_descriptor(
            "graphics_data", subNode, createInfo.hasRasterizationSCI, createInfo.rasterizationSCI,
            createInfo.hasDepthStencilSCI, createInfo.depthStencilSCI, createInfo.hasColourBlendSCI,
            createInfo.colourBlendSCI, createInfo.colourBlendAttachments);
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException(nodeName + "::" + e.what());
        }
    }

    return read;
}

} // namespace

void yaml_read_material_definition(YAML::Node const &node,
                                   foeIdGroupTranslator const *pTranslator,
                                   foeResourceCreateInfoBase **ppCreateInfo) {
    foeMaterialCreateInfo ci;

    yaml_read_material_definition_internal("", node, pTranslator, ci);

    *ppCreateInfo = new foeMaterialCreateInfo(std::move(ci));
}

bool export_yaml_material_definition(foeMaterial const *pMaterial) {
    YAML::Node definition;

    try {
        yaml_write_material_definition("", pMaterial, definition);
    } catch (foeYamlException const &e) {
        FOE_LOG(General, Error, "Failed to export foeMaterial definition: {}", e.what());
        return false;
    }

    YAML::Emitter emitter;
    emitter << definition;

    std::ofstream outFile(std::string{"_"} + std::to_string(pMaterial->getID()) + ".yml",
                          std::ofstream::out);
    if (outFile.is_open()) {
        outFile << emitter.c_str();
        outFile.close();
    } else {
        FOE_LOG(General, Error, "Failed to export foeMaterial: Failed to open output file {}.yml",
                foeIdToString(pMaterial->getID()));
        return false;
    }

    return true;
}