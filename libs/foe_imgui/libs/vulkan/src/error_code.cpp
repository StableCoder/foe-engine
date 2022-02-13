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

struct foeImGuiVkErrCategory : std::error_category {
    const char *name() const noexcept override;
    std::string message(int ev) const override;
};

const char *foeImGuiVkErrCategory::name() const noexcept { return "foeImGuiVkResult"; }

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        return #X;

std::string foeImGuiVkErrCategory::message(int ev) const {
    switch (static_cast<foeImGuiVkResult>(ev)) {
        RESULT_CASE(FOE_IMGUI_VK_SUCCESS)
        // RenderGraph - UI Job
        RESULT_CASE(FOE_IMGUI_VK_GRAPH_UI_COLOUR_TARGET_NOT_IMAGE)
        RESULT_CASE(FOE_IMGUI_VK_GRAPH_UI_COLOUR_TARGET_NOT_MUTABLE)
        RESULT_CASE(FOE_IMGUI_VK_GRAPH_UI_COLOUR_TARGET_MISSING_STATE)

    default:
        if (ev > 0)
            return "(unrecognized positive foeImGuiVkResult value)";
        else
            return "(unrecognized negative foeImGuiVkResult value)";
    }
}

const foeImGuiVkErrCategory cImGuiVkErrCategory{};

} // namespace

std::error_code make_error_code(foeImGuiVkResult e) {
    return {static_cast<int>(e), cImGuiVkErrCategory};
}