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

struct foeGraphicsVkErrCategory : std::error_category {
    const char *name() const noexcept override;
    std::string message(int ev) const override;
};

const char *foeGraphicsVkErrCategory::name() const noexcept { return "foeGraphicsVkResult"; }

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        return #X;

std::string foeGraphicsVkErrCategory::message(int ev) const {
    switch (static_cast<foeGraphicsVkResult>(ev)) {
        RESULT_CASE(FOE_GRAPHICS_VK_SUCCESS)
        RESULT_CASE(FOE_GRAPHICS_VK_INCOMPLETE)
        // RenderTarget
        RESULT_CASE(FOE_GRAPHICS_VK_ERROR_RENDER_TARGET_NO_COMPATIBLE_RENDER_PASS)

    default:
        if (ev > 0)
            return "(unrecognized positive foeGraphicsVkResult value)";
        else
            return "(unrecognized negative foeGraphicsVkResult value)";
    }
}

const foeGraphicsVkErrCategory graphicsVkErrCategory{};

} // namespace

std::error_code make_error_code(foeGraphicsVkResult e) {
    return {static_cast<int>(e), graphicsVkErrCategory};
}