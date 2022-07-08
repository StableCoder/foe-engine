// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef COLLISION_SHAPE_HPP
#define COLLISION_SHAPE_HPP

#include <foe/ecs/group_translator.h>
#include <foe/resource/create_info.h>
#include <yaml-cpp/yaml.h>

struct foeCollisionShapeCreateInfo;

char const *yaml_collision_shape_key();

void yaml_read_collision_shape(YAML::Node const &node,
                               foeEcsGroupTranslator groupTranslator,
                               foeResourceCreateInfo *pCreateInfo);

auto yaml_write_collision_shape(foeCollisionShapeCreateInfo const &data) -> YAML::Node;

#endif // COLLISION_SHAPE_HPP