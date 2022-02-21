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

#ifndef FOE_SIMULATION_CORE_HPP
#define FOE_SIMULATION_CORE_HPP

#include <foe/ecs/id.hpp>
#include <foe/graphics/session.hpp>
#include <foe/simulation/export.h>
#include <foe/simulation/group_data.hpp>

#include <filesystem>
#include <functional>
#include <shared_mutex>
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

struct foeSimulationInitInfo {
    foeGfxSession gfxSession;
    std::function<std::filesystem::path(std::filesystem::path)> externalFileSearchFn;
};

struct foeSimulationLoaderData {
    /// The loader itself
    foeResourceLoaderBase *pLoader;
    /// Maintenance to be performed as part of the regular simulation loop
    void (*pMaintenanceFn)(foeResourceLoaderBase *);
    /// Maintenance to be performed as part of the graphics loop
    void (*pGfxMaintenanceFn)(foeResourceLoaderBase *);
};

struct foeSimulationStateLists {
    foeResourcePoolBase **pResourcePools;
    uint32_t resourcePoolCount;

    foeSimulationLoaderData *pResourceLoaders;
    uint32_t resourceLoaderCount;

    foeComponentPoolBase **pComponentPools;
    uint32_t componentPoolCount;

    foeSystemBase **pSystems;
    uint32_t systemCount;
};

struct foeSimulationState {
    /**
     * @brief Used to synchronize core access to the SimulationState
     *
     * Because this needs to support the possibility of registered core functionality operating
     * asynchronously, when it comes time to deinitialize or destroy functionality, we need to
     * *absolutely* sure that nothing is running asynchronously.
     *
     * To that end, the idea is that any asynchronous task will acquire the shared mutex's 'shared'
     * lock, and when the core needs to modify a SimulationState, then it acquires an exclusive lock
     * once all async tasks are complete, and then performs these modifications.
     *
     * Acquiring the exclusive lock should be an incredibly rare operation.
     */
    std::shared_mutex simSync;

    foeGroupData groupData;

    // Information used to initialize functionality (used when functionality added during runtime)
    foeSimulationInitInfo initInfo{};

    // Resource Data
    foeEditorNameMap *pResourceNameMap;
    std::vector<foeSimulationLoaderData> resourceLoaders;
    std::vector<foeResourcePoolBase *> resourcePools;

    // Entity / Component Data
    foeEditorNameMap *pEntityNameMap;
    std::vector<foeComponentPoolBase *> componentPools;

    // Systems
    std::vector<foeSystemBase *> systems;
};

bool foeSimulationIsInitialized(foeSimulationState const *pSimulationState);

/**
 * @brief Creates a new SimulationState with any registered functionality available
 * @param createInfo Data used during the creation of the simulation and the related functionality
 * @param addNameMaps If true, the optional NameMaps are also made available
 * @return A pointer to a valid SimulationState on success. nullptr otherwise.
 *
 * The 'onCreate' of any previously registered functionality is called on the created simulation.
 */
FOE_SIM_EXPORT auto foeCreateSimulation(bool addNameMaps) -> foeSimulationState *;

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
 * @param FOE_SIMULATION_SUCCESS if successfully destroyed. A descriptive error code otherwise.
 *
 * Returns immediately if the SimulationState had already been initialized.
 *
 * Otherwise iterates through any registered functionality and calls its 'onInitialization'
 * function. If any of these fails, then it backtracks deinitializing anything that did succeed and
 * returns a state fully deinitialized.
 */
FOE_SIM_EXPORT auto foeInitializeSimulation(foeSimulationState *pSimulationState,
                                            foeSimulationInitInfo const *pInitInfo)
    -> std::error_code;

/**
 * @brief Initializes a SimulationState given InitInfo
 * @param pSimulationState SimulatioState to deinitialize
 * @return FOE_SIMULATION_SUCCESS if deinitialization proceeded, otherwise
 * FOE_SIMULATION_ERROR_NOT_INITIALIZED if the simulation has not previously been initialized.
 *
 * Iterates through any registered functionality and calls its 'onDeinitialization' function.
 */
FOE_SIM_EXPORT auto foeDeinitializeSimulation(foeSimulationState *pSimulationState)
    -> std::error_code;

#endif // FOE_SIMULATION_CORE_HPP