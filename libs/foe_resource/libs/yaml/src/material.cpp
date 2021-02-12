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

#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

#include "fragment_descriptor.hpp"

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
        { // Resources Node
            YAML::Node resNode;

            // Fragment Shader
            if (auto *pFragDescriptor = pMaterial->getFragmentDescriptor();
                pFragDescriptor != nullptr) {
                yaml_write_fragment_descriptor_declaration("fragment_descriptor", pFragDescriptor,
                                                           resNode);
            }

            writeNode["resources"] = resNode;
        }

        // Gfx Data Node
        // (Nothing currently)
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

bool yaml_read_material_definition(std::string const &nodeName,
                                   YAML::Node const &node,
                                   std::string &fragmentDescriptor) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    try {
        // Resources
        if (auto resNode = subNode["resources"]; resNode) {
            yaml_read_optional("fragment_descriptor", resNode, fragmentDescriptor);
        }
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return true;
}