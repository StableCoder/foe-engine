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

#include <foe/physics/yaml/rigid_body.hpp>

#include <foe/ecs/yaml/id.hpp>
#include <foe/yaml/parsing.hpp>

auto yaml_read_RigidBody(YAML::Node const &node, foeIdGroupTranslator *pIdGroupTranslator)
    -> foePhysRigidBody {
    foePhysRigidBody rigidBody;

    yaml_read_required("mass", node, rigidBody.mass);
    yaml_read_id_optional("collision_shape", node, pIdGroupTranslator, rigidBody.collisionShape);

    return rigidBody;
}

auto yaml_write_RigidBody(foePhysRigidBody const &data) -> YAML::Node {
    YAML::Node writeNode;

    yaml_write_required("mass", data.mass, writeNode);
    yaml_write_id("collision_shape", data.collisionShape, writeNode);

    return writeNode;
}