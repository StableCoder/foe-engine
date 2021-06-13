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

#ifndef SHADER_HPP
#define SHADER_HPP

#include <foe/resource/shader.hpp>
#include <yaml-cpp/yaml.h>

struct foeIdGroupTranslator;

char const *yaml_shader_key();

void yaml_read_shader(YAML::Node const &node,
                      foeIdGroupTranslator const *pTranslator,
                      foeResourceCreateInfoBase **ppCreateInfo);

auto yaml_write_shader(foeShaderCreateInfo const &data) -> YAML::Node;

#endif // SHADER_HPP