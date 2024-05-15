// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "rigid_body.hpp"

#include <foe/physics/yaml/structs.hpp>

char const *yaml_rigid_body_key() { return "rigid_body"; }

auto yaml_read_rigid_body(YAML::Node const &node,
                          foeEcsGroupTranslator groupTranslator) -> foeRigidBody {
    foeRigidBody rigidBody = {};

    yaml_read_foeRigidBody("", node, groupTranslator, rigidBody);

    return rigidBody;
}

auto yaml_write_rigid_body(foeRigidBody const &data) -> YAML::Node {
    YAML::Node writeNode;

    yaml_write_foeRigidBody("", data, writeNode);

    return writeNode;
}