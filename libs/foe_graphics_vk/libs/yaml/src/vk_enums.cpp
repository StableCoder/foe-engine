// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/yaml/vk_enums.hpp>

#include <foe/external/vk_value_serialization.hpp>
#include <foe/yaml/exception.hpp>
#include <vulkan/vulkan.h>

namespace {

template <typename VkType>
bool yaml_read_vk(std::string const &typeName,
                  std::string const &nodeName,
                  YAML::Node const &node,
                  VkType &data) {
    YAML::Node const &subNode = nodeName.empty() ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    if (vk_parse(typeName.c_str(), subNode.as<std::string>().c_str(), &data) !=
        STEC_VK_SERIALIZATION_RESULT_SUCCESS) {
        if (nodeName.empty()) {
            throw foeYamlException(" - Could not parse node as '" + typeName +
                                   "' with value of: " + subNode.as<std::string>());
        } else {
            throw foeYamlException(nodeName + " - Could not parse node as '" + typeName +
                                   "' with value of: " + subNode.as<std::string>());
        }
    }

    return true;
}

template <typename VkType>
void yaml_write_vk(std::string const &typeName,
                   std::string const &nodeName,
                   VkType const &data,
                   YAML::Node &node) {
    std::string serialized;
    if (vk_serialize(typeName.c_str(), data, &serialized) == STEC_VK_SERIALIZATION_RESULT_SUCCESS) {
        if (nodeName.empty()) {
            node = serialized;
        } else {
            node[nodeName] = serialized;
        }
    } else {
        if (nodeName.empty()) {
            throw foeYamlException(" - Failed to serialize node as '" + typeName + "'");
        } else {
            throw foeYamlException(nodeName + " - Failed to serialize node as '" + typeName + "'");
        }
    }
}

} // namespace

bool yaml_read_VkEnum32(std::string const &typeName,
                        std::string const &nodeName,
                        YAML::Node const &node,
                        uint32_t &data) {
    return yaml_read_vk(typeName, nodeName, node, data);
}

void yaml_write_VkEnum32(std::string const &typeName,
                         std::string const &nodeName,
                         uint32_t const &data,
                         YAML::Node &node) {
    yaml_write_vk(typeName, nodeName, data, node);
}

bool yaml_read_VkEnum64(std::string const &typeName,
                        std::string const &nodeName,
                        YAML::Node const &node,
                        uint64_t &data) {
    return yaml_read_vk(typeName, nodeName, node, data);
}

void yaml_write_VkEnum64(std::string const &typeName,
                         std::string const &nodeName,
                         uint64_t const &data,
                         YAML::Node &node) {
    yaml_write_vk(typeName, nodeName, data, node);
}