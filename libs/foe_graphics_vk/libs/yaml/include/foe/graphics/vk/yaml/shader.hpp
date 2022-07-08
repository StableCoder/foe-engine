// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_YAML_SHADER_HPP
#define FOE_GRAPHICS_VK_YAML_SHADER_HPP

#include <foe/graphics/session.h>
#include <foe/graphics/shader.h>
#include <foe/graphics/vk/shader.hpp>
#include <foe/graphics/vk/yaml/export.h>
#include <vulkan/vulkan.h>
#include <yaml-cpp/yaml.h>

#include <string>

FOE_GFX_VK_YAML_EXPORT
bool yaml_write_gfx_shader(std::string const &nodeName,
                           foeGfxVkShaderCreateInfo const &data,
                           YAML::Node &node);

FOE_GFX_VK_YAML_EXPORT bool yaml_read_gfx_shader(std::string const &nodeName,
                                                 YAML::Node const &node,
                                                 foeGfxVkShaderCreateInfo &data);

#endif // FOE_GRAPHICS_VK_YAML_SHADER_HPP