/*
    Copyright (C) 2021-2022 George Cave.

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