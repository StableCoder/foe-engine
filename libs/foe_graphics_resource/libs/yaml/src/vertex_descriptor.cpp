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

#include "vertex_descriptor.hpp"

#include <foe/ecs/yaml/id.hpp>
#include <foe/graphics/resource/vertex_descriptor_loader.hpp>
#include <foe/graphics/vk/yaml/vertex_descriptor.hpp>
#include <foe/resource/shader.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

namespace {

bool yaml_read_vertex_descriptor_internal(std::string const &nodeName,
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
            read |= yaml_read_id_optional("vertex_shader", resNode, pTranslator,
                                          createInfo.vertexShader);
            read |= yaml_read_id_optional("tessellation_control_shader", resNode, pTranslator,
                                          createInfo.tessellationControlShader);
            read |= yaml_read_id_optional("tessellation_evaluation_shader", resNode, pTranslator,
                                          createInfo.tessellationEvaluationShader);
            read |= yaml_read_id_optional("geometry_shader", resNode, pTranslator,
                                          createInfo.geometryShader);
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

bool yaml_write_vertex_descriptor_internal(std::string const &nodeName,
                                           foeVertexDescriptorCreateInfo const &vertexDescriptor,
                                           YAML::Node &node) {
    YAML::Node writeNode;

    try {
        { // Resources Node
            YAML::Node resNode;

            if (vertexDescriptor.vertexShader != FOE_INVALID_ID) {
                yaml_write_id("vertex_shader", vertexDescriptor.vertexShader, resNode);
            }

            if (vertexDescriptor.tessellationControlShader != FOE_INVALID_ID) {
                yaml_write_id("tessellation_control_shader",
                              vertexDescriptor.tessellationControlShader, resNode);
            }

            if (vertexDescriptor.tessellationEvaluationShader != FOE_INVALID_ID) {
                yaml_write_id("tessellation_evaluation_shader",
                              vertexDescriptor.tessellationEvaluationShader, resNode);
            }

            if (vertexDescriptor.geometryShader != FOE_INVALID_ID) {
                yaml_write_id("geometry_shader", vertexDescriptor.geometryShader, resNode);
            }

            writeNode["resources"] = resNode;
        }
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

char const *yaml_vertex_descriptor_key() { return "vertex_descriptor_v1"; }

void yaml_read_vertex_descriptor(YAML::Node const &node,
                                 foeIdGroupTranslator const *pTranslator,
                                 foeResourceCreateInfoBase **ppCreateInfo) {
    foeVertexDescriptorCreateInfo ci{};

    yaml_read_vertex_descriptor_internal(yaml_vertex_descriptor_key(), node, pTranslator, ci);

    *ppCreateInfo = new foeVertexDescriptorCreateInfo(std::move(ci));
}

auto yaml_write_vertex_descriptor(foeVertexDescriptorCreateInfo const &vertexDescriptor)
    -> YAML::Node {
    YAML::Node definition;

    yaml_write_vertex_descriptor_internal("", vertexDescriptor, definition);

    return definition;
}