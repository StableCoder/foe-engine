/*
    Copyright (C) 2021 George Cave.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

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