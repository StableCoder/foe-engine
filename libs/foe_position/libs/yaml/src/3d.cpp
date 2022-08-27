// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "3d.hpp"

#include <foe/position/yaml/structs.hpp>

char const *yaml_position3d_key() { return "position_3d"; }

auto yaml_read_position3d(YAML::Node const &node) -> foePosition3d {
    foePosition3d position;

    yaml_read_foePosition3d("", node, position);

    return position;
}

auto yaml_write_position3d(foePosition3d const &data) -> YAML::Node {
    YAML::Node writeNode;

    yaml_write_foePosition3d("", data, writeNode);

    return writeNode;
}