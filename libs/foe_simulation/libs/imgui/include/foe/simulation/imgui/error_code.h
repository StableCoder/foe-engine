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

#ifndef FOE_SIMULATION_IMGUI_ERROR_CODE_H
#define FOE_SIMULATION_IMGUI_ERROR_CODE_H

#include <foe/error_code.h>
#include <foe/simulation/imgui/export.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foeSimulationImGuiResult {
    FOE_SIMULATION_IMGUI_SUCCESS = 0,
    FOE_SIMULATION_IMGUI_ERROR_ALL_PARAMETERS_NULL,
    FOE_SIMULATION_IMGUI_ERROR_FUNCTIONALITY_ALREADY_REGISTERED,
    FOE_SIMULATION_IMGUI_ERROR_FUNCTIONALITY_NOT_REGISTERED,

    // Need to have a negative enum value to prevent treatment as a flag
    FOE_SIMULATION_IMGUI_NEGATIVE_VALUE = FOE_RESULT_MIN_ENUM,
} foeSimulationImGuiResult;

FOE_SIM_IMGUI_EXPORT void foeSimulationImGuiResultToString(foeSimulationImGuiResult value,
                                                           char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_SIMULATION_IMGUI_ERROR_CODE_H