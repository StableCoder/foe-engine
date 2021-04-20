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

#include <foe/ecs/yaml/id.hpp>
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
            node = pMaterial->getID();
        } else {
            node[nodeName] = pMaterial->getID();
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
