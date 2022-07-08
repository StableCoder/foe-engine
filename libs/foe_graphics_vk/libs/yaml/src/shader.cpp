// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

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