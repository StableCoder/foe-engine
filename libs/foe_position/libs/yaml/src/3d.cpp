// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "3d.hpp"

#include <foe/yaml/parsing.hpp>

char const *yaml_position3d_key() { return "position_3d"; }

auto yaml_read_position3d(YAML::Node const &node) -> foePosition3d {
    foePosition3d position;

    yaml_read_required("position", node, position.position);
    yaml_read_required("orientation", node, position.orientation);

    return position;
}

auto yaml_write_position3d(foePosition3d const &data) -> YAML::Node {
    YAML::Node writeNode;

    yaml_write_required("position", data.position, writeNode);
    yaml_write_required("orientation", data.orientation, writeNode);

    return writeNode;
}