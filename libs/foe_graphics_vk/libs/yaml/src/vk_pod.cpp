// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/yaml/vk_pod.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/pod.hpp>

bool yaml_read_VkBool32(std::string const &nodeName, YAML::Node const &node, VkBool32 &data) {
    return yaml_read_uint32_t(nodeName, node, data);
}

void yaml_write_VkBool32(std::string const &nodeName, VkBool32 const &data, YAML::Node &node) {
    yaml_write_uint32_t(nodeName, data, node);
}
