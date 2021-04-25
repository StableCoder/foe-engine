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

auto yaml_write_material_definition(foeMaterialCreateInfo const &data,
                                    foeGfxVkFragmentDescriptor *pFragmentDescriptor) -> YAML::Node {
    YAML::Node writeNode;

    try {
        { // Resources Node
            YAML::Node resNode;

            // Fragment Shader
            if (data.fragmentShader != FOE_INVALID_ID) {
                yaml_write_id("fragment_shader", data.fragmentShader, resNode);
            }

            // Image
            if (data.image != FOE_INVALID_ID) {
                yaml_write_id("image", data.image, resNode);
            }

            writeNode["resources"] = resNode;
        }

        // Gfx Data Node
        yaml_write_gfx_fragment_descriptor("graphics_data", pFragmentDescriptor, writeNode);
    } catch (foeYamlException const &e) {
        throw e;
    }

    return writeNode;
}