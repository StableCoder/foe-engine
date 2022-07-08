// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef POSITION_3D_HPP
#define POSITION_3D_HPP

#include <foe/position/component/3d.hpp>
#include <yaml-cpp/yaml.h>

char const *yaml_position3d_key();

auto yaml_read_position3d(YAML::Node const &node) -> foePosition3d;

auto yaml_write_position3d(foePosition3d const &data) -> YAML::Node;

#endif // POSITION_3D_HPP