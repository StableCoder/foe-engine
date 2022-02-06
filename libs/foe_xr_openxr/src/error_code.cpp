/*
    Copyright (C) 2022 George Cave.

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

struct foeXrOpenErrCategory : std::error_category {
    const char *name() const noexcept override;
    std::string message(int ev) const override;
};

const char *foeXrOpenErrCategory::name() const noexcept { return "foeXrOpenResult"; }

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        return #X;

std::string foeXrOpenErrCategory::message(int ev) const {
    switch (static_cast<foeXrOpenResult>(ev)) {
        RESULT_CASE(FOE_OPENXR_SUCCESS)
        RESULT_CASE(FOE_OPENXR_INCOMPLETE)

    default:
        if (ev > 0)
            return "(unrecognized positive foeXrOpenResult value)";
        else
            return "(unrecognized negative foeXrOpenResult value)";
    }
}

const foeXrOpenErrCategory openXrErrCategory{};

} // namespace

std::error_code make_error_code(foeXrOpenResult e) {
    return {static_cast<int>(e), openXrErrCategory};
}