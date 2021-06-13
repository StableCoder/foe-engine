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

#ifndef POSITION_3D_HPP
#define POSITION_3D_HPP

#include <foe/position/component/3d.hpp>
#include <yaml-cpp/yaml.h>

char const* yaml_position3d_key();

auto yaml_read_position3d(YAML::Node const &node) -> foePosition3d;

auto yaml_write_position3d(foePosition3d const &data) -> YAML::Node;

#endif // POSITION_3D_HPP