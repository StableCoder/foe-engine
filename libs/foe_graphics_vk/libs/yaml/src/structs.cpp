// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/yaml/structs.hpp>

#include <foe/graphics/vk/cleanup.h>
#include <foe/graphics/vk/shader.h>
#include <foe/graphics/vk/yaml/vk_structs.hpp>
#include <foe/graphics/yaml/enums.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/glm.hpp>
#include <foe/yaml/pod.hpp>
#include <vk_struct_compare.h>

bool yaml_read_foeGfxVkShaderCreateInfo(std::string const &nodeName,
                                        YAML::Node const &node,
                                        foeGfxVkShaderCreateInfo &data) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        return false;
    }

    foeGfxVkShaderCreateInfo newData = {};
    try {
        // foeBuiltinDescriptorSetLayoutFlags - builtinSetLayouts
        yaml_read_foeBuiltinDescriptorSetLayoutFlags("builtin_set_layouts", readNode,
                                                     newData.builtinSetLayouts);

        // VkDescriptorSetLayoutCreateInfo - descriptorSetLayoutCI
        yaml_read_VkDescriptorSetLayoutCreateInfo("descriptor_set_layout_ci", readNode,
                                                  newData.descriptorSetLayoutCI);

        // VkPushConstantRange - pushConstantRange
        yaml_read_VkPushConstantRange("push_constant_range", readNode, newData.pushConstantRange);
    } catch (foeYamlException const &e) {
        cleanup_foeGfxVkShaderCreateInfo(&newData);

        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    data = newData;
    return true;
}

void yaml_write_foeGfxVkShaderCreateInfo(std::string const &nodeName,
                                         foeGfxVkShaderCreateInfo const &data,
                                         YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // foeBuiltinDescriptorSetLayoutFlags - builtinSetLayouts
        if (data.builtinSetLayouts != 0) {
            yaml_write_foeBuiltinDescriptorSetLayoutFlags("builtin_set_layouts",
                                                          data.builtinSetLayouts, writeNode);
        }

        // VkDescriptorSetLayoutCreateInfo - descriptorSetLayoutCI
        if (VkDescriptorSetLayoutCreateInfo tmp = {};
            !compare_VkDescriptorSetLayoutCreateInfo(&data.descriptorSetLayoutCI, &tmp)) {
            yaml_write_VkDescriptorSetLayoutCreateInfo("descriptor_set_layout_ci",
                                                       data.descriptorSetLayoutCI, writeNode);
        }

        // VkPushConstantRange - pushConstantRange
        if (VkPushConstantRange tmp = {};
            !compare_VkPushConstantRange(&data.pushConstantRange, &tmp)) {
            yaml_write_VkPushConstantRange("push_constant_range", data.pushConstantRange,
                                           writeNode);
        }
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}
