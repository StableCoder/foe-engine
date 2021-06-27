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

struct foePositionYamlErrorCategory : std::error_category {
    const char *name() const noexcept override;
    std::string message(int ev) const override;
};

const char *foePositionYamlErrorCategory::name() const noexcept { return "foePositionYamlResult"; }

#define ENUM_CASE(X)                                                                               \
    case X:                                                                                        \
        return #X;

std::string foePositionYamlErrorCategory::message(int ev) const {
    switch (static_cast<foePositionYamlResult>(ev)) {
        ENUM_CASE(FOE_POSITION_YAML_SUCCESS)
        // Position3D
        ENUM_CASE(FOE_POSITION_YAML_ERROR_FAILED_TO_REGISTER_3D_IMPORTER)
        ENUM_CASE(FOE_POSITION_YAML_ERROR_FAILED_TO_REGISTER_3D_EXPORTER)

    default:
        if (ev > 0)
            return "(unrecognized positive foePositionYamlResult value)";
        else
            return "(unrecognized negative foePositionYamlResult value)";
    }
}

const foePositionYamlErrorCategory errorCategory{};

} // namespace

std::error_code make_error_code(foePositionYamlResult e) {
    return {static_cast<int>(e), errorCategory};
}