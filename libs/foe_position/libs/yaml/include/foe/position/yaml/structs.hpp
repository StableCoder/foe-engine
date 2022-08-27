// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_POSITION_YAML_STRUCTS_HPP
#define FOE_POSITION_YAML_STRUCTS_HPP

#include <foe/ecs/group_translator.h>
#include <foe/position/yaml/export.h>
#include <yaml-cpp/yaml.h>

#include <string>

struct foePosition3d;

FOE_POSITION_YAML_EXPORT bool yaml_read_foePosition3d(std::string const &nodeName,
                                                      YAML::Node const &node,
                                                      foePosition3d &data);

FOE_POSITION_YAML_EXPORT void yaml_write_foePosition3d(std::string const &nodeName,
                                                       foePosition3d const &data,
                                                       YAML::Node &node);

#endif // FOE_POSITION_YAML_STRUCTS_HPP
