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

struct foePhysicsYamlErrorCategory : std::error_category {
    const char *name() const noexcept override;
    std::string message(int ev) const override;
};

const char *foePhysicsYamlErrorCategory::name() const noexcept { return "foePhysicsYamlResult"; }

#define RESULT_CASE(X)                                                                             \
    case #X:                                                                                       \
        return ##X;

std::string foePhysicsYamlErrorCategory::message(int ev) const {
    switch (static_cast<foePhysicsYamlResult>(ev)) {
    case FOE_PHYSICS_YAML_SUCCESS:
        return "FOE_PHYSICS_YAML_SUCCESS";

    case FOE_PHYSICS_YAML_ERROR_UNSPECIFIED:
        return "FOE_PHYSICS_YAML_ERROR_UNSPECIFIED";

    case FOE_PHYSICS_YAML_ERROR_COLLISION_SHAPE_POOL_NOT_FOUND:
        return "FOE_PHYSICS_YAML_ERROR_COLLISION_SHAPE_POOL_NOT_FOUND";

    case FOE_PHYSICS_YAML_ERROR_COLLISION_SHAPE_ALREADY_EXISTS:
        return "FOE_PHYSICS_YAML_ERROR_COLLISION_SHAPE_ALREADY_EXISTS";

    default:
        if (ev > 0)
            return "(unrecognized positive foePhysicsYamlResult value)";
        else
            return "(unrecognized negative foePhysicsYamlResult value)";
    }
}

const foePhysicsYamlErrorCategory errorCategory{};

} // namespace

std::error_code make_error_code(foePhysicsYamlResult e) {
    return {static_cast<int>(e), errorCategory};
}