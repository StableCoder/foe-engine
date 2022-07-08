// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef SHADER_HPP
#define SHADER_HPP

#include <foe/ecs/group_translator.h>
#include <foe/resource/create_info.h>
#include <yaml-cpp/yaml.h>

struct foeShaderCreateInfo;

char const *yaml_shader_key();

void yaml_read_shader(YAML::Node const &node,
                      foeEcsGroupTranslator groupTranslator,
                      foeResourceCreateInfo *pCreateInfo);

auto yaml_write_shader(foeShaderCreateInfo const &data) -> YAML::Node;

#endif // SHADER_HPP