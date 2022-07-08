// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

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