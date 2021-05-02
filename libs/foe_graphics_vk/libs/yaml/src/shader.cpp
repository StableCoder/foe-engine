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

#include <foe/graphics/vk/yaml/shader.hpp>

#include <foe/graphics/yaml/builtin_descriptor_sets.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

#include "shader.hpp"

bool yaml_write_gfx_shader(std::string const &nodeName,
                           foeGfxVkShaderCreateInfo const &data,
                           YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // Builtin Descriptor Set Layouts
        yaml_write_builtin_descriptor_set_layouts("builtin_descriptor_set_layouts",
                                                  data.builtinSetLayouts, writeNode);

        // Descriptor Set Layout
        yaml_write_optional("descriptor_set_layout", VkDescriptorSetLayoutCreateInfo{},
                            data.descriptorSetLayoutCI, writeNode);

        // Push Constant Range
        yaml_write_optional("push_constant_range", VkPushConstantRange{}, data.pushConstantRange,
                            writeNode);
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

bool yaml_read_gfx_shader(std::string const &nodeName,
                          YAML::Node const &node,
                          foeGfxVkShaderCreateInfo &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    foeGfxVkShaderCreateInfo tempData{};
    try {
        // Builtin Descriptor Set Layouts
        yaml_read_builtin_descriptor_set_layouts("builtin_descriptor_set_layouts", subNode,
                                                 tempData.builtinSetLayouts);

        // DescriptorSetLayouts
        yaml_read_optional("descriptor_set_layout", subNode, tempData.descriptorSetLayoutCI);

        // Push Constant Range
        yaml_read_optional("push_constant_range", subNode, tempData.pushConstantRange);
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    data = tempData;
    return true;
}