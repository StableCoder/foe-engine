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

#ifndef FOE_SIMULATION_REGISTRATION_HPP
#define FOE_SIMULATION_REGISTRATION_HPP

#include <foe/simulation/export.h>

#include <system_error>

struct foeSimulationState;
struct foeSimulationInitInfo;
struct foeSimulationStateLists;

struct foeSimulationFunctionalty {
    /// Called on any created SimulationState, to create related data pools, uninitialized systems.
    void (*onCreate)(foeSimulationState *);
    /// Called when destroying any SimulationState to destroy related data pools and systems.
    void (*onDestroy)(foeSimulationState *);

    /// To be called after onCreate when a Simulation is being initialized to start actually running
    /// a Simulation
    void (*onInitialization)(foeSimulationInitInfo const *, foeSimulationStateLists const *);
    /// Called before onDestroy to safely destory any running state for an active SimulationState
    void (*onDeinitialization)(foeSimulationState const *);

    bool operator==(foeSimulationFunctionalty const &) const noexcept;
    bool operator!=(foeSimulationFunctionalty const &) const noexcept;
};

/**
 * @brief Attempts to register a set of simulation functionality globally
 * @param functionality Set of functionality to be registered
 * @return FOE_SIMULATION_SUCCESS on successful registration. A descriptive error code otherwise.
 *
 * First checks to see if the *exact* same set of functions already exists. If it does this returns
 * false. The same function may be used as part of a different set which would be counted as
 * 'different'.
 *
 * If there are any created simulations, then the provided 'onCreate' function is called on them. If
 * any simulations have been initialized prior, then the 'onInitialize' function is called on them.
 */
FOE_SIM_EXPORT auto foeRegisterFunctionality(foeSimulationFunctionalty const &functionality)
    -> std::error_code;

/**
 * @brief Attempts to deregister a set of simulation functionality globally
 * @param functionality Set of functionality to be deregistered
 * @return FOE_SIMULATION_SUCCESS on successful deregistration. A descriptive error code otherwise.
 *
 * If the functionality is found and to be deregistered, it will first iterate through all created
 * simulations and call 'onDeinitialization' and 'onDestroy' to remove the functionality before
 * finally returning.
 */
FOE_SIM_EXPORT auto foeDeregisterFunctionality(foeSimulationFunctionalty const &functionality)
    -> std::error_code;

#endif // FOE_SIMULATION_REGISTRATION_HPP