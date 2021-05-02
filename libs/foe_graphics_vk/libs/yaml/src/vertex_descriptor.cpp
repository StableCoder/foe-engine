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

#include <foe/graphics/vk/yaml/vertex_descriptor.hpp>

#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

bool yaml_write_gfx_vertex_descriptor(std::string const &nodeName,
                                      foeGfxVertexDescriptor const *pVertexDescriptor,
                                      YAML::Node &node) {
    YAML::Node writeNode;

    bool dataWritten = true;

    try {
        { // Vertex Input SCI
            YAML::Node vertexInputNode;
            yaml_write_optional("", VkPipelineVertexInputStateCreateInfo{},
                                pVertexDescriptor->mVertexInputSCI, vertexInputNode);

            // Input Bindings
            YAML::Node bindingsNode;
            for (auto &it : pVertexDescriptor->mVertexInputBindings) {
                YAML::Node node;
                yaml_write_required("", it, node);

                bindingsNode.push_back(node);
            }
            vertexInputNode["input_bindings"] = bindingsNode;

            // Input Attributes
            YAML::Node attributesNode;
            for (auto &it : pVertexDescriptor->mVertexInputAttributes) {
                YAML::Node node;
                yaml_write_required("", it, node);

                attributesNode.push_back(node);
            }
            vertexInputNode["input_attributes"] = attributesNode;

            // Write out
            writeNode["vertex_input"] = vertexInputNode;
        }

        // Input Assembly
        yaml_write_optional("input_assembly", VkPipelineInputAssemblyStateCreateInfo{},
                            pVertexDescriptor->mInputAssemblySCI, writeNode);

        // Tessellation
        if (pVertexDescriptor->getTessellationSCI() != nullptr) {
            yaml_write_optional("tessellation", VkPipelineTessellationStateCreateInfo{},
                                *pVertexDescriptor->getTessellationSCI(), writeNode);
        }
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    if (dataWritten) {
        if (nodeName.empty()) {
            node = writeNode;
        } else {
            node[nodeName] = writeNode;
        }
    }

    return dataWritten;
}

bool yaml_read_gfx_vertex_descriptor(
    std::string const &nodeName,
    YAML::Node const &node,
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
        { // Vertex Input SCI
            auto vertexInputNode = subNode["vertex_input"];
            vertexInputSCI = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            };
            yaml_read_optional("", vertexInputNode, vertexInputSCI);

            // Input Bindings
            if (auto bindingsNode = vertexInputNode["input_bindings"]; bindingsNode) {
                for (auto it = bindingsNode.begin(); it != bindingsNode.end(); ++it) {
                    VkVertexInputBindingDescription description;
                    yaml_read_required("", *it, description);

                    inputBindings.emplace_back(description);
                }
            }

            // Input Attributes
            if (auto attributesNode = vertexInputNode["input_attributes"]; attributesNode) {
                for (auto it = attributesNode.begin(); it != attributesNode.end(); ++it) {
                    VkVertexInputAttributeDescription description;
                    yaml_read_required("", *it, description);

                    inputAttributes.emplace_back(description);
                }
            }
        }

        // Input Assembly
        inputAssemblySCI = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        };
        yaml_read_optional("input_assembly", subNode, inputAssemblySCI);

        // Tessellation
        tessellationSCI = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
        };
        yaml_read_optional("tessellation", subNode, tessellationSCI);
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return true;
}