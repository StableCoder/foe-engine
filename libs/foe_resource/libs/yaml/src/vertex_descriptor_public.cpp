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
#include <foe/graphics/yaml/vertex_descriptor.hpp>
#include <foe/log.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

#include "shader.hpp"

#include <fstream>

namespace {

bool yaml_read_vertex_descriptor_definition(
    std::string const &nodeName,
    YAML::Node const &node,
    foeId &vertexShader,
    foeId &tessellationControlShader,
    foeId &tessellationEvaluationShader,
    foeId &geometryShader,
    VkPipelineVertexInputStateCreateInfo &vertexInputSCI,
    std::vector<VkVertexInputBindingDescription> &inputBindings,
    std::vector<VkVertexInputAttributeDescription> &inputAttributes,
    VkPipelineInputAssemblyStateCreateInfo &inputAssemblySCI,
    VkPipelineTessellationStateCreateInfo &tessellationSCI) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    try {
        // Resources
        if (auto resNode = subNode["resources"]; resNode) {
            yaml_read_optional("vertex_shader", resNode, vertexShader);
            yaml_read_optional("tessellation_control_shader", resNode, tessellationControlShader);
            yaml_read_optional("tessellation_evaluation_shader", resNode,
                               tessellationEvaluationShader);
            yaml_read_optional("geometry_shader", resNode, geometryShader);
        }

        // Graphics Data
        yaml_read_gfx_vertex_descriptor("graphics_data", subNode, vertexInputSCI, inputBindings,
                                        inputAttributes, inputAssemblySCI, tessellationSCI);
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return true;
}

} // namespace

bool yaml_read_vertex_descriptor_definition(YAML::Node const &node,
                                            foeVertexDescriptorCreateInfo &createInfo) {
    try {
        yaml_read_vertex_descriptor_definition(
            "", node, createInfo.vertexShader, createInfo.tessellationControlShader,
            createInfo.tessellationEvaluationShader, createInfo.geometryShader,
            createInfo.vertexInputSCI, createInfo.inputBindings, createInfo.inputAttributes,
            createInfo.inputAssemblySCI, createInfo.tessellationSCI);
    } catch (foeYamlException const &e) {
        FOE_LOG(General, Error, "Failed to import foeFragmentDescriptor definition: {}", e.what());
        return false;
    }

    return true;
}

void yaml_read_vertex_descriptor_definition2(YAML::Node const &node,
                                             foeResourceCreateInfoBase **ppCreateInfo) {
    foeVertexDescriptorCreateInfo ci;

    yaml_read_vertex_descriptor_definition(node, ci);

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
                yaml_write_shader_declaration("vertex_shader", pShader, resNode);
            }

            if (auto *pShader = pVertexDescriptor->getTessellationControlShader();
                pShader != nullptr) {
                yaml_write_shader_declaration("tessellation_control_shader", pShader, resNode);
            }

            if (auto *pShader = pVertexDescriptor->getTessellationEvaluationShader();
                pShader != nullptr) {
                yaml_write_shader_declaration("tessellation_evaluation_shader", pShader, resNode);
            }

            if (auto *pShader = pVertexDescriptor->getGeometryShader(); pShader != nullptr) {
                yaml_write_shader_declaration("geometry_shader", pShader, resNode);
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

bool export_yaml_vertex_descriptor_definition(foeVertexDescriptor const *pVertexDescriptor) {
    YAML::Node definition;

    try {
        yaml_write_vertex_descriptor_definition("", pVertexDescriptor, definition);
    } catch (foeYamlException const &e) {
        FOE_LOG(General, Error, "Failed to export foeVertexDescriptor definition: {}", e.what());
    }

    YAML::Emitter emitter;
    emitter << definition;

    std::ofstream outFile(std::string{"_"} + std::to_string(pVertexDescriptor->getID()) + ".yml",
                          std::ofstream::out);
    if (outFile.is_open()) {
        outFile << emitter.c_str();
        outFile.close();
    } else {
        FOE_LOG(General, Error,
                "Failed to export foeVertexDescriptor: Failed to open output file {}.yml",
                foeIdToString(pVertexDescriptor->getID()));
        return false;
    }

    return true;
}