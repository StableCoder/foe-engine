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

#ifndef FOE_POSITION_YAML_COMPONENT_3D_HPP
#define FOE_POSITION_YAML_COMPONENT_3D_HPP

#include <foe/position/component/3d.hpp>
#include <foe/position/yaml/export.h>
#include <yaml-cpp/yaml.h>

FOE_POSITION_YAML_EXPORT auto yaml_read_Position3D(YAML::Node const &node) -> foePosition3d;

FOE_POSITION_YAML_EXPORT auto yaml_write_Position3D(foePosition3d const &data) -> YAML::Node;

#endif // FOE_POSITION_YAML_COMPONENT_3D_HPP