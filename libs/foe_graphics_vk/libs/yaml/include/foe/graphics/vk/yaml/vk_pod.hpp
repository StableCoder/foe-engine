// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_YAML_VK_POD_HPP
#define FOE_GRAPHICS_VK_YAML_VK_POD_HPP

#include <foe/graphics/vk/yaml/export.h>
#include <vulkan/vulkan.h>
#include <yaml-cpp/yaml.h>

#define POD_YAML_DECLARATION(T)                                                                    \
    FOE_GFX_VK_YAML_EXPORT                                                                         \
    bool yaml_read_##T(std::string const &nodeName, YAML::Node const &node, T &data);              \
                                                                                                   \
    FOE_GFX_VK_YAML_EXPORT                                                                         \
    void yaml_write_##T(std::string const &nodeName, T const &data, YAML::Node &node);

POD_YAML_DECLARATION(VkBool32)

#undef POD_YAML_DECLARATION

#endif // FOE_GRAPHICS_VK_YAML_VK_POD_HPP