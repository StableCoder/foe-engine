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

#include "error_code.hpp"

namespace {

struct foeYamlResourceErrorCategory : std::error_category {
    const char *name() const noexcept override;
    std::string message(int ev) const override;
};

const char *foeYamlResourceErrorCategory::name() const noexcept { return "foeResourceYamlResult"; }

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        return #X;

std::string foeYamlResourceErrorCategory::message(int ev) const {
    switch (static_cast<foeResourceYamlResult>(ev)) {
        RESULT_CASE(FOE_RESOURCE_YAML_SUCCESS)
        RESULT_CASE(FOE_RESOURCE_YAML_ERROR_UNSPECIFIED)
        RESULT_CASE(FOE_RESOURCE_YAML_ERROR_ARMATURE_POOL_NOT_FOUND)
        RESULT_CASE(FOE_RESOURCE_YAML_ERROR_ARMATURE_RESOURCE_ALREADY_EXISTS)
        RESULT_CASE(FOE_RESOURCE_YAML_ERROR_IMAGE_POOL_NOT_FOUND)
        RESULT_CASE(FOE_RESOURCE_YAML_ERROR_IMAGE_RESOURCE_ALREADY_EXISTS)
        RESULT_CASE(FOE_RESOURCE_YAML_ERROR_MATERIAL_POOL_NOT_FOUND)
        RESULT_CASE(FOE_RESOURCE_YAML_ERROR_MATERIAL_RESOURCE_ALREADY_EXISTS)
        RESULT_CASE(FOE_RESOURCE_YAML_ERROR_MESH_POOL_NOT_FOUND)
        RESULT_CASE(FOE_RESOURCE_YAML_ERROR_MESH_RESOURCE_ALREADY_EXISTS)
        RESULT_CASE(FOE_RESOURCE_YAML_ERROR_SHADER_POOL_NOT_FOUND)
        RESULT_CASE(FOE_RESOURCE_YAML_ERROR_SHADER_RESOURCE_ALREADY_EXISTS)
        RESULT_CASE(FOE_RESOURCE_YAML_ERROR_VERTEX_DESCRIPTOR_POOL_NOT_FOUND)
        RESULT_CASE(FOE_RESOURCE_YAML_ERROR_VERTEX_DESCRIPTOR_RESOURCE_ALREADY_EXISTS)

    default:
        if (ev > 0)
            return "(unrecognized positive foeResourceYamlResult value)";
        else
            return "(unrecognized negative foeResourceYamlResult value)";
    }
}

const foeYamlResourceErrorCategory yamlResourceErrorCategory{};

} // namespace

std::error_code make_error_code(foeResourceYamlResult e) {
    return {static_cast<int>(e), yamlResourceErrorCategory};
}