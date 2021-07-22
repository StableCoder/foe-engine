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

#include <foe/resource/error_code.hpp>

namespace {

struct foeResourceErrCategory : std::error_category {
    const char *name() const noexcept override;
    std::string message(int ev) const override;
};

const char *foeResourceErrCategory::name() const noexcept { return "foeResourceResult"; }

std::string foeResourceErrCategory::message(int ev) const {
    switch (static_cast<foeResourceResult>(ev)) {
    case FOE_RESOURCE_SUCCESS:
        return "FOE_RESOURCE_SUCCESS";
    case FOE_RESOURCE_ERROR_ALREADY_INITIALIZED:
        return "FOE_RESOURCE_ERROR_ALREADY_INITIALIZED";
    case FOE_RESOURCE_ERROR_IMPORT_FAILED:
        return "FOE_RESOURCE_ERROR_IMPORT_FAILED";
    case FOE_RESOURCE_ERROR_FAILED_TO_FIND_COMPATIBLE_LOADER:
        return "FOE_RESOURCE_ERROR_FAILED_TO_FIND_COMPATIBLE_LOADER";

    default:
        if (ev > 0)
            return "(unrecognized positive foeResourceResult value)";
        else
            return "(unrecognized negative foeResourceResult value)";
    }
}

const foeResourceErrCategory resourceErrCategory{};

} // namespace

std::error_code make_error_code(foeResourceResult e) {
    return {static_cast<int>(e), resourceErrCategory};
}