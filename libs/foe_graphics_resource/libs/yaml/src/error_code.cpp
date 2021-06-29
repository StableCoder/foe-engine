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

struct foeGraphicsResourceYamlErrorCategory : std::error_category {
    const char *name() const noexcept override;
    std::string message(int ev) const override;
};

const char *foeGraphicsResourceYamlErrorCategory::name() const noexcept {
    return "foeGraphicsResourceYamlResult";
}

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        return #X;

std::string foeGraphicsResourceYamlErrorCategory::message(int ev) const {
    switch (static_cast<foeGraphicsResourceYamlResult>(ev)) {
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_YAML_SUCCESS)

        // Material Resource
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_YAML_ERROR_MATERIAL_POOL_NOT_FOUND)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_YAML_ERROR_MATERIAL_RESOURCE_ALREADY_EXISTS)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_MATERIAL_IMPORTER)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_MATERIAL_EXPORTER)

    default:
        if (ev > 0)
            return "(unrecognized positive foeGraphicsResourceYamlResult value)";
        else
            return "(unrecognized negative foeGraphicsResourceYamlResult value)";
    }
}

const foeGraphicsResourceYamlErrorCategory errorCategory{};

} // namespace

std::error_code make_error_code(foeGraphicsResourceYamlResult e) {
    return {static_cast<int>(e), errorCategory};
}