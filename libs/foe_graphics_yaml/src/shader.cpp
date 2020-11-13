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

#include <foe/graphics/yaml/shader.hpp>

#include <foe/graphics/descriptor_set_layout_pool.hpp>
#include <foe/graphics/shader.hpp>
#include <foe/graphics/shader_pool.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

#include <limits>

bool yaml_read_shader_declaration(std::string const &nodeName,
                                  YAML::Node const &node,
                                  foeShaderPool *pShaderPool,
                                  foeShader **pShader) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    try {
        std::string shaderName;
        yaml_read_required("", subNode, shaderName);

        *pShader = pShaderPool->create(shaderName);
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return true;
}

bool yaml_write_shader_declaration(std::string const &nodeName,
                                   foeShader const *pShader,
                                   YAML::Node &node) {
    try {
        if (nodeName.empty()) {
            node = pShader->name;
        } else {
            node[nodeName] = pShader->name;
        }
    } catch (...) {
        throw foeYamlException(nodeName + " - Failed to serialize 'foeShader'");
    }

    return true;
}

bool yaml_read_shader_definition(std::string const &nodeName,
                                 YAML::Node const &node,
                                 foeDescriptorSetLayoutPool *pDescriptorSetLayoutPool,
                                 foeShader *pShader) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    try {
        // Builtin Descriptor Set Layouts
        foeBuiltinDescriptorSetLayoutFlags builtinSetLayouts = 0;
        if (auto builtinsNode = subNode["builtin_descriptor_set_layouts"]; builtinsNode) {
            for (auto it = builtinsNode.begin(); it != builtinsNode.end(); ++it) {
                std::string builtinName;
                yaml_read_required("", *it, builtinName);

                builtinSetLayouts |= to_builtin_set_layout(builtinName);
            }
        }

        // DescriptorSetLayouts
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        };
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;

        if (auto layoutNode = subNode["descriptor_set_layout"]; layoutNode) {
            yaml_read_optional("", subNode, descriptorSetLayoutCI);

            if (auto bindingsNode = subNode["bindings"]; bindingsNode) {
                for (auto const &it : bindingsNode) {
                    VkDescriptorSetLayoutBinding data{};
                    yaml_read_required("", it, data);

                    setLayoutBindings.emplace_back(data);
                }
            }
        }
        descriptorSetLayoutCI.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorSetLayoutCI.pBindings = setLayoutBindings.data();

        // Push Constant Range
        VkPushConstantRange pushConstantRange{};
        yaml_read_optional("push_constant_range", subNode, pushConstantRange);

        // All done, export to shader
        pShader->builtinSetLayouts = builtinSetLayouts;
        pShader->layout = pDescriptorSetLayoutPool->get(&descriptorSetLayoutCI);
        pShader->pushConstantRange = pushConstantRange;
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return true;
}

bool yaml_write_shader_definition(std::string const &nodeName,
                                  foeBuiltinDescriptorSets const *pBuiltinDescriptorSets,
                                  foeDescriptorSetLayoutPool *pDescriptorSetLayoutPool,
                                  foeShader const *pShader,
                                  YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // Builtin Descriptor Set Layouts
        auto builtinSetLayouts = pShader->builtinSetLayouts;
        YAML::Node builtinSetLayoutsNode;
        for (size_t i = 0; i < std::numeric_limits<foeBuiltinDescriptorSetLayoutFlags>::digits &&
                           builtinSetLayouts != 0;
             ++i) {
            foeBuiltinDescriptorSetLayoutFlagBits setFlag =
                static_cast<foeBuiltinDescriptorSetLayoutFlagBits>(1 << i);

            if ((builtinSetLayouts & setFlag) != 0) {
                builtinSetLayoutsNode.push_back(to_string(setFlag));
            }
        }
        if (builtinSetLayoutsNode.begin() != builtinSetLayoutsNode.end()) {
            writeNode["builtin_descriptor_set_layouts"] = builtinSetLayoutsNode;
        }

        // Descriptor Set Layout
        if (pShader->layout != VK_NULL_HANDLE &&
            pShader->layout != pBuiltinDescriptorSets->getDummyLayout()) {
            VkDescriptorSetLayoutCreateInfo layoutCI;
            std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

            if (!pDescriptorSetLayoutPool->getCI(pShader->layout, layoutCI, layoutBindings)) {
                throw foeYamlException("descriptor_set_layout - Could not retireve info from "
                                       "foeDescriptorSetLayoutPool");
            }

            YAML::Node layoutNode;
            yaml_write_optional("", VkDescriptorSetLayoutCreateInfo{}, layoutCI, layoutNode);

            if (!layoutBindings.empty()) {
                YAML::Node bindingsNode;

                for (auto const &it : layoutBindings) {
                    YAML::Node binding;
                    yaml_write_required("", it, binding);
                    bindingsNode.push_back(binding);
                }

                layoutNode["bindings"] = bindingsNode;
            }

            writeNode["descriptor_set_layout"] = layoutNode;
        }

        // Push Constant Range
        yaml_write_optional("push_constant_range", VkPushConstantRange{},
                            pShader->pushConstantRange, writeNode);
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }

    return true;
}