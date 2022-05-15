/*
    Copyright (C) 2021-2022 George Cave.

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

#ifndef RIGID_BODY_HPP
#define RIGID_BODY_HPP

#include <foe/ecs/group_translator.h>
#include <foe/physics/component/rigid_body.hpp>
#include <yaml-cpp/yaml.h>

char const *yaml_rigid_body_key();

auto yaml_read_rigid_body(YAML::Node const &node, foeEcsGroupTranslator groupTranslator)
    -> foeRigidBody;

auto yaml_write_rigid_body(foeRigidBody const &data) -> YAML::Node;

#endif // RIGID_BODY_HPP