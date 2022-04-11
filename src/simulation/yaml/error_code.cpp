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

#include "error_code.hpp"

namespace {

struct ErrorCategory : std::error_category {
    const char *name() const noexcept override;
    std::string message(int ev) const override;
};

const char *ErrorCategory::name() const noexcept { return "foeBringupYamlResult"; }

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        return #X;

std::string ErrorCategory::message(int ev) const {
    switch (static_cast<foeBringupYamlResult>(ev)) {
        RESULT_CASE(FOE_BRINGUP_YAML_SUCCESS)
        RESULT_CASE(FOE_BRINGUP_YAML_ERROR_UNSPECIFIED)

        // Importers
        RESULT_CASE(FOE_BRINGUP_YAML_ERROR_ARMATURE_POOL_NOT_FOUND)
        RESULT_CASE(FOE_BRINGUP_YAML_ERROR_ARMATURE_RESOURCE_ALREADY_EXISTS)

        RESULT_CASE(FOE_BRINGUP_YAML_ERROR_FAILED_TO_REGISTER_ARMATURE_IMPORTER)
        RESULT_CASE(FOE_BRINGUP_YAML_ERROR_FAILED_TO_REGISTER_ARMATURE_STATE_IMPORTER)
        RESULT_CASE(FOE_BRINGUP_YAML_ERROR_FAILED_TO_REGISTER_RENDER_STATE_IMPORTER)
        RESULT_CASE(FOE_BRINGUP_YAML_ERROR_FAILED_TO_REGISTER_CAMERA_IMPORTER)

        // Exporters
        RESULT_CASE(FOE_BRINGUP_YAML_ERROR_FAILED_TO_REGISTER_RESOURCE_EXPORTERS)
        RESULT_CASE(FOE_BRINGUP_YAML_ERROR_FAILED_TO_REGISTER_COMPONENT_EXPORTERS)

    default:
        if (ev > 0)
            return "(unrecognized positive foeBringupYamlResult value)";
        else
            return "(unrecognized negative foeBringupYamlResult value)";
    }
}

const ErrorCategory errorCategory{};

} // namespace

std::error_code make_error_code(foeBringupYamlResult e) {
    return {static_cast<int>(e), errorCategory};
}