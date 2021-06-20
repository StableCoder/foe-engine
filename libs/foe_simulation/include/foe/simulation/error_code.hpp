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

#ifndef FOE_SIMULATION_ERROR_CODE_HPP
#define FOE_SIMULATION_ERROR_CODE_HPP

#include <foe/simulation/export.h>

#include <system_error>

enum foeSimulationResult : int {
    FOE_SIMULATION_SUCCESS = 0,
    FOE_SIMULATION_ERROR_FUNCTIONALITY_ALREADY_REGISTERED,
    FOE_SIMULATION_ERROR_FUNCTIONALITY_NOT_REGISTERED,
    FOE_SIMULATION_ERROR_SIMULATION_NOT_REGISTERED,
};

namespace std {
template <>
struct is_error_code_enum<foeSimulationResult> : true_type {};
} // namespace std

FOE_SIM_EXPORT std::error_code make_error_code(foeSimulationResult);

#endif // FOE_SIMULATION_ERROR_CODE_HPP