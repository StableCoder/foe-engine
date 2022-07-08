// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

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