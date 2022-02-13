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

struct foeWsiErrCategory : std::error_category {
    const char *name() const noexcept override;
    std::string message(int ev) const override;
};

const char *foeWsiErrCategory::name() const noexcept { return "foeWsiResult"; }

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        return #X;

std::string foeWsiErrCategory::message(int ev) const {
    switch (static_cast<foeWsiResult>(ev)) {
        RESULT_CASE(FOE_WSI_SUCCESS)
        RESULT_CASE(FOE_WSI_ERROR_FAILED_TO_INITIALIZE_BACKEND)
        RESULT_CASE(FOE_WSI_ERROR_FAILED_TO_CREATE_WINDOW)
        RESULT_CASE(FOE_WSI_ERROR_VULKAN_NOT_SUPPORTED)

    default:
        if (ev > 0)
            return "(unrecognized positive foeWsiResult value)";
        else
            return "(unrecognized negative foeWsiResult value)";
    }
}

#undef RESULT_CASE

const foeWsiErrCategory errorCategory{};

} // namespace

std::error_code make_error_code(foeWsiResult e) { return {static_cast<int>(e), errorCategory}; }