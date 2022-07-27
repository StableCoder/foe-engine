// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

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
            for (uint32_t i = 0; i < pVertexDescriptor->vertexInputBindingCount; ++i) {
                YAML::Node node;
                yaml_write_required("", pVertexDescriptor->pVertexInputBindings[i], node);

                bindingsNode.push_back(node);
            }
            vertexInputNode["input_bindings"] = bindingsNode;

            // Input Attributes
            YAML::Node attributesNode;
            for (uint32_t i = 0; i < pVertexDescriptor->vertexInputAttributeCount; ++i) {
                YAML::Node node;
                yaml_write_required("", pVertexDescriptor->pVertexInputAttributes[i], node);

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

bool yaml_read_gfx_vertex_descriptor(std::string const &nodeName,
                                     YAML::Node const &node,
                                     VkPipelineVertexInputStateCreateInfo &vertexInputSCI,
                                     uint32_t &inputBindingCount,
                                     VkVertexInputBindingDescription *&pInputBindings,
                                     uint32_t &inputAttributeCount,
                                     VkVertexInputAttributeDescription *&pInputAttributes,
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
                inputBindingCount = bindingsNode.size();
                pInputBindings = (VkVertexInputBindingDescription *)calloc(
                    inputBindingCount, sizeof(VkVertexInputBindingDescription));

                size_t count = 0;
                for (auto it = bindingsNode.begin(); it != bindingsNode.end(); ++it) {
                    VkVertexInputBindingDescription description;
                    yaml_read_required("", *it, description);

                    pInputBindings[count] = description;
                    ++count;
                }
            }

            // Input Attributes
            if (auto attributesNode = vertexInputNode["input_attributes"]; attributesNode) {
                inputAttributeCount = attributesNode.size();
                pInputAttributes = (VkVertexInputAttributeDescription *)calloc(
                    inputAttributeCount, sizeof(VkVertexInputAttributeDescription));

                size_t count = 0;
                for (auto it = attributesNode.begin(); it != attributesNode.end(); ++it) {
                    VkVertexInputAttributeDescription description;
                    yaml_read_required("", *it, description);

                    pInputAttributes[count] = description;
                    ++count;
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