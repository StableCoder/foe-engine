// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_SIMULATION_RESULT_H
#define FOE_SIMULATION_RESULT_H

#include <foe/result.h>
#include <foe/simulation/export.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foeSimulationResult {
    FOE_SIMULATION_SUCCESS = 0,
    FOE_SIMULATION_ERROR_OUT_OF_MEMORY,
    FOE_SIMULATION_ERROR_CONTENT_NOT_FOUND,
    FOE_SIMULATION_ERROR_NOT_REGISTERED,
    FOE_SIMULATION_ERROR_NO_LOADER_FOUND,
    // Functionality Registration
    FOE_SIMULATION_ERROR_ID_INVALID,
    FOE_SIMULATION_ERROR_ID_ALREADY_IN_USE,
    // Types
    FOE_SIMULATION_ERROR_TYPE_NOT_FOUND,
    FOE_SIMULATION_ERROR_TYPE_ALREADY_EXISTS,
    // Simulation
    FOE_SIMULATION_ERROR_CREATING_RESOURCE_POOL,
    FOE_SIMULATION_ERROR_SIMULATION_ALREADY_INITIALIZED,
    FOE_SIMULATION_ERROR_SIMULATION_NOT_INITIALIZED,
    FOE_SIMULATION_ERROR_SIMULATION_GRAPHICS_ALREADY_INITIALIZED,
    FOE_SIMULATION_ERROR_SIMULATION_GRAPHICS_NOT_INITIALIZED,

    // Need to have a negative enum value to prevent treatment as a flag
    FOE_SIMULATION_ERROR_NEGATIVE_VALUE = FOE_RESULT_MIN_ENUM,
} foeSimulationResult;

FOE_SIM_EXPORT void foeSimulationResultToString(foeSimulationResult value,
                                                char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_SIMULATION_RESULT_H