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

struct foeSimulationImGuiErrCategory : std::error_category {
    const char *name() const noexcept override;
    std::string message(int ev) const override;
};

const char *foeSimulationImGuiErrCategory::name() const noexcept { return "foeSimulationResult"; }

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        return #X;

std::string foeSimulationImGuiErrCategory::message(int ev) const {
    switch (static_cast<foeSimulationImGuiResult>(ev)) {
        RESULT_CASE(FOE_SIMULATION_IMGUI_SUCCESS)
        RESULT_CASE(FOE_SIMULATION_IMGUI_ERROR_ALL_PARAMETERS_NULL)
        RESULT_CASE(FOE_SIMULATION_IMGUI_ERROR_FUNCTIONALITY_ALREADY_REGISTERED)
        RESULT_CASE(FOE_SIMULATION_IMGUI_ERROR_FUNCTIONALITY_NOT_REGISTERED)

    default:
        if (ev > 0)
            return "(unrecognized positive foeSimulationImGuiResult value)";
        else
            return "(unrecognized negative foeSimulationImGuiResult value)";
    }
}

const foeSimulationImGuiErrCategory errorCategory{};

} // namespace

std::error_code make_error_code(foeSimulationImGuiResult e) {
    return {static_cast<int>(e), errorCategory};
}