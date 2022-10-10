// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef YAML_ARMATURE_HPP
#define YAML_ARMATURE_HPP

#include <foe/ecs/group_translator.h>
#include <foe/resource/create_info.h>
#include <yaml-cpp/yaml.h>

struct foeArmatureCreateInfo;

char const *yaml_armature_key();

void yaml_read_armature(YAML::Node const &node,
                        foeEcsGroupTranslator groupTranslator,
                        foeResourceCreateInfo *pCreateInfo);

auto yaml_write_armature(foeArmatureCreateInfo const &data) -> YAML::Node;

#endif // YAML_ARMATURE_HPP