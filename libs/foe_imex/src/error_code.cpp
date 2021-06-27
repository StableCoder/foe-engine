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

struct foeGraphicsResourceErrCategory : std::error_category {
    const char *name() const noexcept override;
    std::string message(int ev) const override;
};

const char *foeGraphicsResourceErrCategory::name() const noexcept { return "foeImexResult"; }

#define ENUM_CASE(X)                                                                               \
    case X:                                                                                        \
        return #X;

std::string foeGraphicsResourceErrCategory::message(int ev) const {
    switch (static_cast<foeImexResult>(ev)) {
        ENUM_CASE(FOE_IMEX_SUCCESS)
        ENUM_CASE(FOE_IMEX_ERROR_FUNCTIONALITY_ALREADY_REGISTERED)
        ENUM_CASE(FOE_IMEX_ERROR_EXPORTER_ALREADY_REGISTERED)

    default:
        if (ev > 0)
            return "(unrecognized positive foeImexResult value)";
        else
            return "(unrecognized negative foeImexResult value)";
    }
}

const foeGraphicsResourceErrCategory graphicsResourceErrCategory{};

} // namespace

std::error_code make_error_code(foeImexResult e) {
    return {static_cast<int>(e), graphicsResourceErrCategory};
}