// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "rigid_body.hpp"

#include <foe/ecs/yaml/id.hpp>
#include <foe/yaml/parsing.hpp>

char const *yaml_rigid_body_key() { return "rigid_body"; }

auto yaml_read_rigid_body(YAML::Node const &node, foeEcsGroupTranslator groupTranslator)
    -> foeRigidBody {
    foeRigidBody rigidBody = {};

    yaml_read_required("mass", node, rigidBody.mass);
    yaml_read_id_optional("collision_shape", node, groupTranslator, rigidBody.collisionShape);

    return rigidBody;
}

auto yaml_write_rigid_body(foeRigidBody const &data) -> YAML::Node {
    YAML::Node writeNode;

    yaml_write_required("mass", data.mass, writeNode);
    yaml_write_id("collision_shape", data.collisionShape, writeNode);

    return writeNode;
}