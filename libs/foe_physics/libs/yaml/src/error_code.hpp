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

#ifndef ERROR_CODE_HPP
#define ERROR_CODE_HPP

#include <system_error>

enum foePhysicsYamlResult {
    FOE_PHYSICS_YAML_SUCCESS = 0,
    // CollisionShape Resource
    FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_COLLISION_SHAPE_IMPORTER,
    FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_COLLISION_SHAPE_EXPORTER,
    FOE_PHYSICS_YAML_ERROR_COLLISION_SHAPE_POOL_NOT_FOUND,
    FOE_PHYSICS_YAML_ERROR_COLLISION_SHAPE_ALREADY_EXISTS,
    // RigidBody Component
    FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_RIGID_BODY_IMPORTER,
    FOE_PHYSICS_YAML_ERROR_FAILED_TO_REGISTER_RIGID_BODY_EXPORTER,
};

namespace std {
template <>
struct is_error_code_enum<foePhysicsYamlResult> : true_type {};
} // namespace std

std::error_code make_error_code(foePhysicsYamlResult);

#endif // ERROR_CODE_HPP