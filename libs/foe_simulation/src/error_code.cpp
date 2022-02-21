/*
    Copyright (C) 2021-2022 George Cave.

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

struct foeSimulationErrCategory : std::error_category {
    const char *name() const noexcept override;
    std::string message(int ev) const override;
};

const char *foeSimulationErrCategory::name() const noexcept { return "foeSimulationResult"; }

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        return #X;

std::string foeSimulationErrCategory::message(int ev) const {
    switch (static_cast<foeSimulationResult>(ev)) {
        RESULT_CASE(FOE_SIMULATION_SUCCESS)
        RESULT_CASE(FOE_SIMULATION_ERROR_FUNCTIONALITY_ALREADY_REGISTERED)
        RESULT_CASE(FOE_SIMULATION_ERROR_FUNCTIONALITY_NOT_REGISTERED)
        RESULT_CASE(FOE_SIMULATION_ERROR_SIMULATION_NOT_REGISTERED)
        RESULT_CASE(FOE_SIMULATION_ERROR_GFX_SESSION_NOT_PROVIDED)
        RESULT_CASE(FOE_SIMULATION_ERROR_ALREADY_INITIALIZED)
        RESULT_CASE(FOE_SIMULATION_ERROR_NOT_INITIALIZED)

    default:
        if (ev > 0)
            return "(unrecognized positive foeSimulationResult value)";
        else
            return "(unrecognized negative foeSimulationResult value)";
    }
}

const foeSimulationErrCategory errorCategory{};

} // namespace

std::error_code make_error_code(foeSimulationResult e) {
    return {static_cast<int>(e), errorCategory};
}