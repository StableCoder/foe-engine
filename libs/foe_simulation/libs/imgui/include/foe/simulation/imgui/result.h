// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_SIMULATION_IMGUI_RESULT_H
#define FOE_SIMULATION_IMGUI_RESULT_H

#include <foe/result.h>
#include <foe/simulation/imgui/export.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foeSimulationImGuiResult {
    FOE_SIMULATION_IMGUI_SUCCESS = 0,
    FOE_SIMULATION_IMGUI_ERROR_ALL_PARAMETERS_NULL = -1000014001,
    FOE_SIMULATION_IMGUI_ERROR_FUNCTIONALITY_ALREADY_REGISTERED = -1000014002,
    FOE_SIMULATION_IMGUI_ERROR_FUNCTIONALITY_NOT_REGISTERED = -1000014003,
} foeSimulationImGuiResult;

FOE_SIM_IMGUI_EXPORT
void foeSimulationImGuiResultToString(foeSimulationImGuiResult value,
                                      char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_SIMULATION_IMGUI_RESULT_H