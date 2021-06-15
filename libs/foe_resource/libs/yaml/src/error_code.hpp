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

enum foeResourceYamlResult {
    FOE_RESOURCE_YAML_SUCCESS = 0,
    FOE_RESOURCE_YAML_ERROR_UNSPECIFIED,
    FOE_RESOURCE_YAML_ERROR_ARMATURE_POOL_NOT_FOUND,
    FOE_RESOURCE_YAML_ERROR_ARMATURE_RESOURCE_ALREADY_EXISTS,
    FOE_RESOURCE_YAML_ERROR_IMAGE_POOL_NOT_FOUND,
    FOE_RESOURCE_YAML_ERROR_IMAGE_RESOURCE_ALREADY_EXISTS,
    FOE_RESOURCE_YAML_ERROR_MATERIAL_POOL_NOT_FOUND,
    FOE_RESOURCE_YAML_ERROR_MATERIAL_RESOURCE_ALREADY_EXISTS,
    FOE_RESOURCE_YAML_ERROR_MESH_POOL_NOT_FOUND,
    FOE_RESOURCE_YAML_ERROR_MESH_RESOURCE_ALREADY_EXISTS,
    FOE_RESOURCE_YAML_ERROR_SHADER_POOL_NOT_FOUND,
    FOE_RESOURCE_YAML_ERROR_SHADER_RESOURCE_ALREADY_EXISTS,
    FOE_RESOURCE_YAML_ERROR_VERTEX_DESCRIPTOR_POOL_NOT_FOUND,
    FOE_RESOURCE_YAML_ERROR_VERTEX_DESCRIPTOR_RESOURCE_ALREADY_EXISTS,
};

namespace std {
template <>
struct is_error_code_enum<foeResourceYamlResult> : true_type {};
} // namespace std

std::error_code make_error_code(foeResourceYamlResult);

#endif // ERROR_CODE_HPP