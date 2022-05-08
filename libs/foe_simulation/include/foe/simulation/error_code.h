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

#ifndef FOE_SIMULATION_ERROR_CODE_H
#define FOE_SIMULATION_ERROR_CODE_H

#ifdef __cplusplus
extern "C" {
#endif

enum foeSimulationResult {
    FOE_SIMULATION_SUCCESS = 0,
    FOE_SIMULATION_ERROR_NOT_REGISTERED,
    FOE_SIMULATION_ERROR_NO_LOADER_FOUND,
    // Functionality Registration
    FOE_SIMULATION_ERROR_ID_INVALID,
    FOE_SIMULATION_ERROR_ID_ALREADY_IN_USE,
    // Types
    FOE_SIMULATION_ERROR_TYPE_NOT_FOUND,
    FOE_SIMULATION_ERROR_TYPE_ALREADY_EXISTS,
    // Simulation
    FOE_SIMULATION_ERROR_SIMULATION_ALREADY_INITIALIZED,
    FOE_SIMULATION_ERROR_SIMULATION_NOT_INITIALIZED,
    FOE_SIMULATION_ERROR_SIMULATION_GRAPHICS_ALREADY_INITIALIZED,
    FOE_SIMULATION_ERROR_SIMULATION_GRAPHICS_NOT_INITIALIZED,
};

#ifdef __cplusplus
}
#endif

#endif // FOE_SIMULATION_ERROR_CODE_H