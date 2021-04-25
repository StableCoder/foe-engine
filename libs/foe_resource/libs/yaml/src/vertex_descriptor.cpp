/*
    Copyright (C) 2020 George Cave.

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

#include <foe/resource/yaml/vertex_descriptor.hpp>

#include <foe/ecs/id.hpp>
#include <foe/ecs/yaml/id.hpp>
#include <foe/graphics/yaml/vertex_descriptor.hpp>
#include <foe/log.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

#include "shader.hpp"

#include <fstream>

namespace {

bool yaml_read_vertex_descriptor_definition_internal(std::string const &nodeName,
                                                     YAML::Node const &node,
                                                     foeIdGroupTranslator const *pTranslator,
                                                     foeVertexDescriptorCreateInfo &createInfo) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    bool read{false};

    try {
        // Resources
        if (auto resNode = subNode["resources"]; resNode) {
            read |= yaml_read_id_optional("vertex_shader", resNode, pTranslator, foeIdTypeResource,
                                          createInfo.vertexShader);
            read |= yaml_read_id_optional("tessellation_control_shader", resNode, pTranslator,
                                          foeIdTypeResource, createInfo.tessellationControlShader);
            read |=
                yaml_read_id_optional("tessellation_evaluation_shader", resNode, pTranslator,
                                      foeIdTypeResource, createInfo.tessellationEvaluationShader);
            read |= yaml_read_id_optional("geometry_shader", resNode, pTranslator,
                                          foeIdTypeResource, createInfo.geometryShader);
        }

        // Graphics Data
        read |= yaml_read_gfx_vertex_descriptor(
            "graphics_data", subNode, createInfo.vertexInputSCI, createInfo.inputBindings,
            createInfo.inputAttributes, createInfo.inputAssemblySCI, createInfo.tessellationSCI);
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.whatStr()};
        }
        return false;
    }

    return read;
}

} // namespace

void yaml_read_vertex_descriptor_definition(YAML::Node const &node,
                                            foeIdGroupTranslator const *pTranslator,
                                            foeResourceCreateInfoBase **ppCreateInfo) {
    foeVertexDescriptorCreateInfo ci;

    yaml_read_vertex_descriptor_definition_internal("", node, pTranslator, ci);

    *ppCreateInfo = new foeVertexDescriptorCreateInfo(std::move(ci));
}

namespace {

bool yaml_write_vertex_descriptor_definition(std::string const &nodeName,
                                             foeVertexDescriptor const *pVertexDescriptor,
                                             YAML::Node &node) {
    YAML::Node writeNode;

    try {
        { // Resources Node
            YAML::Node resNode;

            if (auto *pShader = pVertexDescriptor->getVertexShader(); pShader != nullptr) {
                yaml_write_id("vertex_shader", pShader->getID(), resNode);
            }

            if (auto *pShader = pVertexDescriptor->getTessellationControlShader();
                pShader != nullptr) {
                yaml_write_id("tessellation_control_shader", pShader->getID(), resNode);
            }

            if (auto *pShader = pVertexDescriptor->getTessellationEvaluationShader();
                pShader != nullptr) {
                yaml_write_id("tessellation_evaluation_shader", pShader->getID(), resNode);
            }

            if (auto *pShader = pVertexDescriptor->getGeometryShader(); pShader != nullptr) {
                yaml_write_id("geometry_shader", pShader->getID(), resNode);
            }

            writeNode["resources"] = resNode;
        }

        // Graphics Data Node
        yaml_write_gfx_vertex_descriptor("graphics_data",
                                         pVertexDescriptor->getGfxVertexDescriptor(), writeNode);
    } catch (...) {
        throw foeYamlException(nodeName +
                               " - Failed to serialize 'foeVertexDescriptor' definition");
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }

    return true;
}

} // namespace

auto yaml_write_vertex_descriptor_definition(foeVertexDescriptor const *pVertexDescriptor)
    -> YAML::Node {
    YAML::Node definition;

    yaml_write_vertex_descriptor_definition("", pVertexDescriptor, definition);

    return definition;
}