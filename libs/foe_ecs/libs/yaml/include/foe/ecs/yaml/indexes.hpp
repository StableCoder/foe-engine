// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_ECS_YAML_INDEXES_HPP
#define FOE_ECS_YAML_INDEXES_HPP

#include <foe/ecs/indexes.h>
#include <foe/ecs/yaml/export.h>
#include <yaml-cpp/yaml.h>

FOE_ECS_YAML_EXPORT
void yaml_read_indexes(std::string const &nodeName, YAML::Node const &node, foeEcsIndexes indexes);

FOE_ECS_YAML_EXPORT
void yaml_write_indexes(std::string const &nodeName, foeEcsIndexes indexes, YAML::Node &node);

#endif // FOE_ECS_YAML_INDEXES_HPP