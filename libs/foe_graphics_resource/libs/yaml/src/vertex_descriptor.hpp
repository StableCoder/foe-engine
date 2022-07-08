// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef VERTEX_DESCRIPTOR_HPP
#define VERTEX_DESCRIPTOR_HPP

#include <foe/ecs/group_translator.h>
#include <foe/resource/create_info.h>
#include <yaml-cpp/yaml.h>

struct foeVertexDescriptorCreateInfo;

char const *yaml_vertex_descriptor_key();

void yaml_read_vertex_descriptor(YAML::Node const &node,
                                 foeEcsGroupTranslator groupTranslator,
                                 foeResourceCreateInfo *pCreateInfo);

auto yaml_write_vertex_descriptor(foeVertexDescriptorCreateInfo const &data) -> YAML::Node;

#endif // VERTEX_DESCRIPTOR_HPP