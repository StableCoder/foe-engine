/*
    Copyright (C) 2022 George Cave.

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

#include <foe/graphics/vk/yaml/vk_enum.hpp>

#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>
#include <vk_value_serialization.hpp>
#include <vulkan/vulkan.h>

namespace {

template <typename VkType>
bool yaml_read_optional_vk(std::string const &typeName,
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
void yaml_read_required_vk(std::string const &typeName,
                           std::string const &nodeName,
                           YAML::Node const &node,
                           VkType &data) {
    if (!yaml_read_optional_vk(typeName, nodeName, node, data)) {
        if (nodeName.empty()) {
            throw foeYamlException(" - Required node not found to parse as '" + typeName + "'");
        } else {
            throw foeYamlException(nodeName + " - Required node not found to parse as '" +
                                   typeName + "'");
        }
    }
}

template <typename VkType>
void yaml_write_required_vk(std::string const &typeName,
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

template <typename VkType>
bool yaml_write_optional_vk(std::string const &typeName,
                            std::string const &nodeName,
                            VkType const &defaultData,
                            VkType const &data,
                            YAML::Node &node) {
    if (data == defaultData) {
        return false;
    }

    yaml_write_required_vk(typeName, nodeName, data, node);

    return true;
}

} // namespace

void yaml_read_required_VkEnum32(std::string const &typeName,
                                 std::string const &nodeName,
                                 YAML::Node const &node,
                                 uint32_t &data) {
    yaml_read_required_vk(typeName, nodeName, node, data);
}

bool yaml_read_optional_VkEnum32(std::string const &typeName,
                                 std::string const &nodeName,
                                 YAML::Node const &node,
                                 uint32_t &data) {
    return yaml_read_optional_vk(typeName, nodeName, node, data);
}

void yaml_write_required_VkEnum32(std::string const &typeName,
                                  std::string const &nodeName,
                                  uint32_t const &data,
                                  YAML::Node &node) {
    yaml_write_required_vk(typeName, nodeName, data, node);
}

bool yaml_write_optional_VkEnum32(std::string const &typeName,
                                  std::string const &nodeName,
                                  uint32_t const &defaultData,
                                  uint32_t const &data,
                                  YAML::Node &node) {
    return yaml_write_optional_vk(typeName, nodeName, defaultData, data, node);
}

void yaml_read_required_VkEnum64(std::string const &typeName,
                                 std::string const &nodeName,
                                 YAML::Node const &node,
                                 uint64_t &data) {
    yaml_read_required_vk(typeName, nodeName, node, data);
}

bool yaml_read_optional_VkEnum64(std::string const &typeName,
                                 std::string const &nodeName,
                                 YAML::Node const &node,
                                 uint64_t &data) {
    return yaml_read_optional_vk(typeName, nodeName, node, data);
}

void yaml_write_required_VkEnum64(std::string const &typeName,
                                  std::string const &nodeName,
                                  uint64_t const &data,
                                  YAML::Node &node) {
    yaml_write_required_vk(typeName, nodeName, data, node);
}

bool yaml_write_optional_VkEnum64(std::string const &typeName,
                                  std::string const &nodeName,
                                  uint64_t const &defaultData,
                                  uint64_t const &data,
                                  YAML::Node &node) {
    return yaml_write_optional_vk(typeName, nodeName, defaultData, data, node);
}