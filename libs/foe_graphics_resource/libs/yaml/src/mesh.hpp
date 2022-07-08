// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MESH_HPP
#define MESH_HPP

#include <foe/ecs/group_translator.h>
#include <foe/resource/create_info.h>
#include <yaml-cpp/yaml.h>

struct foeMeshCreateInfo;

char const *yaml_mesh_key();

void yaml_read_mesh(YAML::Node const &node,
                    foeEcsGroupTranslator groupTranslator,
                    foeResourceCreateInfo *pCreateInfo);

auto yaml_write_mesh(foeMeshCreateInfo const &data) -> YAML::Node;

#endif // MESH_HPP