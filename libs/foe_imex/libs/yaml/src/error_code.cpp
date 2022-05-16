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

const char *ErrorCategory::name() const noexcept { return "foeImexYamlResult"; }

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        return #X;

std::string ErrorCategory::message(int ev) const {
    switch (static_cast<foeImexYamlResult>(ev)) {
        RESULT_CASE(FOE_IMEX_YAML_SUCCESS)
        RESULT_CASE(FOE_IMEX_YAML_INCOMPLETE)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_FUNCTIONALITY_ALREADY_REGISTERED)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_FUNCTIONALITY_NOT_REGISTERED)
        // Importer
        RESULT_CASE(FOE_IMEX_YAML_ERROR_PATH_NOT_DIRECTORY)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_DEPENDENCIES_FILE_NOT_EXIST)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_DEPENDENCIES_FILE_NOT_REGULAR_FILE)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_FAILED_TO_READ_DEPENDENCIES)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_RESOURCE_INDEX_FILE_NOT_EXIST)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_RESOURCE_INDEX_FILE_NOT_REGULAR_FILE)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_ENTITY_INDEX_FILE_NOT_EXIST)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_ENTITY_INDEX_FILE_NOT_REGULAR_FILE)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_RESOURCE_DIRECTORY_NOT_DIRECTORY)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_ENTITY_DIRECTORY_NOT_DIRECTORY)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_EXTERNAL_DIRECTORY_NOT_DIRECTORY)
        // Exporter
        RESULT_CASE(FOE_IMEX_YAML_ERROR_EXPORTER_ALREADY_REGISTERED)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_EXPORTER_NOT_REGISTERED)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_DESTINATION_NOT_DIRECTORY)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_FAILED_TO_PERFORM_FILESYSTEM_OPERATION)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_DEPENDENCIES)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_RESOURCE_INDEX_DATA)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_COMPONENT_INDEX_DATA)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_RESOURCE_DATA)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_COMPONENT_DATA)

    default:
        if (ev > 0)
            return "(unrecognized positive foeImexYamlResult value)";
        else
            return "(unrecognized negative foeImexYamlResult value)";
    }
}

const ErrorCategory errorCategory{};

} // namespace

std::error_code make_error_code(foeImexYamlResult e) {
    return {static_cast<int>(e), errorCategory};
}