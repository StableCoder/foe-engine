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

struct foePhysicsErrCategory : std::error_category {
    const char *name() const noexcept override;
    std::string message(int ev) const override;
};

const char *foePhysicsErrCategory::name() const noexcept { return "foePhysicsResult"; }

std::string foePhysicsErrCategory::message(int ev) const {
    switch (static_cast<foePhysicsResult>(ev)) {
    case FOE_PHYSICS_SUCCESS:
        return "FOE_RESOURCE_SUCCESS";
    case FOE_PHYSICS_ERROR_IMPORT_FAILED:
        return "FOE_PHYSICS_ERROR_IMPORT_FAILED";

    default:
        if (ev > 0)
            return "(unrecognized positive foePhysicsResult value)";
        else
            return "(unrecognized negative foePhysicsResult value)";
    }
}

const foePhysicsErrCategory physicsErrCategory{};

} // namespace

std::error_code make_error_code(foePhysicsResult e) {
    return {static_cast<int>(e), physicsErrCategory};
}