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

#ifndef FOE_SIMULATION_CORE_HPP
#define FOE_SIMULATION_CORE_HPP

#include <foe/simulation/export.h>

#include <foe/ecs/id.hpp>
#include <foe/graphics/session.hpp>

#include <filesystem>
#include <functional>
#include <system_error>

/**
 * The Simulation 'core' is a static global registry of all simulation functionality that has been
 * added that is available.
 *
 * The idea is that, at runtime, libraries can be dynamically loaded and unloaded, and the
 * functionality specific to that library can be added/removed to all SimulationState sets, and be
 * used for dealing with the creation/initialization and deinitialization/destruction of these sets
 * safely.
 */

struct foeResourceCreateInfoBase;
struct foeResourcePoolBase;
struct foeResourceLoaderBase;
struct foeComponentPoolBase;
struct foeSystemBase;

struct foeSimulationState;

struct foeSimulationInitInfo {
    foeGfxSession gfxSession;
    std::function<foeResourceCreateInfoBase *(foeId)> resourceDefinitionImportFn;
    std::function<std::filesystem::path(std::filesystem::path)> externalFileSearchFn;
    std::function<void(std::function<void()>)> asyncJobFn;
};

struct foeSimulationStateLists {
    foeResourcePoolBase **pResourcePools;
    uint32_t resourcePoolCount;

    foeResourceLoaderBase **pResourceLoaders;
    uint32_t resourceLoaderCount;

    foeComponentPoolBase **pComponentPools;
    uint32_t componentPoolCount;

    foeSystemBase **pSystems;
    uint32_t systemCount;
};

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

/**
 * @brief Creates a new SimulationState with any registered functionality available
 * @param addNameMaps If true, the optional NameMaps are also made available
 * @return A pointer to a valid SimulationState on success. nullptr otherwise.
 *
 * The 'onCreate' of any previously registered functionality is called on the created simulation.
 */
FOE_SIM_EXPORT foeSimulationState *foeCreateSimulation(bool addNameMaps);

/**
 * @brief Attempts to destroy a given SimulationState
 * @param pSimulationState Object to attempt to destroy
 * @return FOE_SIMULATION_SUCCESS if successfully destroyed. A descriptive error code otherwise.
 *
 * The given SimulationState is first checked to see if it was created by the given registry and if
 * not, returns false without interacting with it.
 *
 * If the simulation is valid and was previously initialized, the 'onDeinitialize' is called.
 * 'onDestroy' is called last and the object is then freed.
 */
FOE_SIM_EXPORT auto foeDestroySimulation(foeSimulationState *pSimulationState) -> std::error_code;

/**
 * @brief Initializes a SimulationState given InitInfo
 * @param pSimulationState SimulatioState to initialize
 * @param pInitInfo Required information used to initialize a SimulationState
 *
 * Returns immediately if the SimulationState had already been initialized.
 *
 * Otherwise iterates through any registered functionality and calls its 'onInitialization'
 * function.
 */
FOE_SIM_EXPORT void foeInitializeSimulation(foeSimulationState *pSimulationState,
                                            foeSimulationInitInfo const *pInitInfo);

/**
 * @brief Initializes a SimulationState given InitInfo
 * @param pSimulationState SimulatioState to deinitialize
 *
 * Iterates through any registered functionality and calls its 'onDeinitialization' function.
 */
FOE_SIM_EXPORT void foeDeinitializeSimulation(foeSimulationState *pSimulationState);

#endif // FOE_SIMULATION_CORE_HPP