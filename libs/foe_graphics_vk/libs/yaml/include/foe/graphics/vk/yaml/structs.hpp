// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_YAML_STRUCTS_HPP
#define FOE_GRAPHICS_VK_YAML_STRUCTS_HPP

#include <foe/ecs/group_translator.h>
#include <foe/graphics/vk/yaml/export.h>
#include <yaml-cpp/yaml.h>

#include <string>

struct foeGfxVkShaderCreateInfo;

FOE_GFX_VK_YAML_EXPORT
bool yaml_read_foeGfxVkShaderCreateInfo(std::string const &nodeName,
                                        YAML::Node const &node,
                                        foeGfxVkShaderCreateInfo &data);

FOE_GFX_VK_YAML_EXPORT
void yaml_write_foeGfxVkShaderCreateInfo(std::string const &nodeName,
                                         foeGfxVkShaderCreateInfo const &data,
                                         YAML::Node &node);

#endif // FOE_GRAPHICS_VK_YAML_STRUCTS_HPP
