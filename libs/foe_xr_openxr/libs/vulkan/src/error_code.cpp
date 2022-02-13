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

struct foeOpenXrVkErrCategory : std::error_category {
    const char *name() const noexcept override;
    std::string message(int ev) const override;
};

const char *foeOpenXrVkErrCategory::name() const noexcept { return "foeOpenXrVkResult"; }

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        return #X;

std::string foeOpenXrVkErrCategory::message(int ev) const {
    switch (static_cast<foeOpenXrVkResult>(ev)) {
        RESULT_CASE(FOE_OPENXR_VK_SUCCESS)

    default:
        if (ev > 0)
            return "(unrecognized positive foeOpenXrVkResult value)";
        else
            return "(unrecognized negative foeOpenXrVkResult value)";
    }
}

const foeOpenXrVkErrCategory cOpenXrVkErrCategory{};

} // namespace

std::error_code make_error_code(foeOpenXrVkResult e) {
    return {static_cast<int>(e), cOpenXrVkErrCategory};
}