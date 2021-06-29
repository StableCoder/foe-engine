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

enum foeGraphicsResourceYamlResult {
    FOE_GRAPHICS_RESOURCE_YAML_SUCCESS = 0,
    // Material Resource
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_MATERIAL_POOL_NOT_FOUND,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_MATERIAL_RESOURCE_ALREADY_EXISTS,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_MATERIAL_IMPORTER,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_MATERIAL_EXPORTER,
};

namespace std {
template <>
struct is_error_code_enum<foeGraphicsResourceYamlResult> : true_type {};
} // namespace std

std::error_code make_error_code(foeGraphicsResourceYamlResult);

#endif // ERROR_CODE_HPP