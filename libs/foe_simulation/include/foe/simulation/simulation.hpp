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

#include <foe/ecs/id.h>
#include <foe/graphics/session.hpp>
#include <foe/resource/pool.h>
#include <foe/resource/resource.h>
#include <foe/simulation/export.h>
#include <foe/simulation/group_data.hpp>
#include <foe/simulation/type_defs.h>

#include <filesystem>
#include <functional>
#include <shared_mutex>

/**
 * The Simulation 'core' is a static global registry of all simulation functionality that has been
 * added that is available.
 *
 * The idea is that, at runtime, libraries can be dynamically loaded and unloaded, and the
 * functionality specific to that library can be added/removed to all SimulationState sets, and be
 * used for dealing with the creation/initialization and deinitialization/destruction of these sets
 * safely.
 *
 * Certain items are considered *optional* to a simulation and can be de/initialized separately,
 * such as graphics.
 */

struct foeResourceLoaderBase;
struct foeComponentPoolBase;
struct foeSystemBase;

struct foeSimulationInitInfo {
    std::function<std::filesystem::path(std::filesystem::path)> externalFileSearchFn;
};

struct foeSimulationLoaderData {
    foeSimulationStructureType sType;
    size_t refCount;
    size_t initCount;
    size_t gfxInitCount;
    /// The loader itself
    void *pLoader;
    /// Function that returns whether the loader can process a given ResourceCreateInfo type
    bool (*pCanProcessCreateInfoFn)(foeResourceCreateInfo);
    /// Function to be called to load a resource with a compatible ResourceCreateInfo
    void (*pLoadFn)(void *, foeResource, foeResourceCreateInfo, PFN_foeResourcePostLoad *);
    /// Maintenance to be performed as part of the regular simulation loop
    void (*pMaintenanceFn)(void *);
    /// Maintenance to be performed as part of the graphics loop
    void (*pGfxMaintenanceFn)(void *);
};

struct foeSimulationComponentPoolData {
    foeSimulationStructureType sType;
    size_t refCount;
    void *pComponentPool;
    void (*pMaintenanceFn)(void *);
};

struct foeSimulationSystemData {
    foeSimulationStructureType sType;
    size_t refCount;
    size_t initCount;
    size_t gfxInitCount;
    void *pSystem;
};

struct foeSimulation {
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

    // Optional Simulation
    foeGfxSession gfxSession;

    // Resource Data
    foeEditorNameMap *pResourceNameMap;
    foeResourcePool resourcePool;
    std::vector<foeSimulationLoaderData> resourceLoaders;

    // Entity / Component Data
    foeEditorNameMap *pEntityNameMap;
    std::vector<foeSimulationComponentPoolData> componentPools;

    // Systems
    std::vector<foeSimulationSystemData> systems;
};

/// Return if a simulation has been successfully initialized
bool foeSimulationIsInitialized(foeSimulation const *pSimulation);

/// Returns if a simulation has had graphics successfully initialized
bool foeSimulationIsGraphicsInitialzied(foeSimulation const *pSimulation);

/**
 * @brief Creates a new SimulationState with any registered functionality available
 * @param addNameMaps If true, the optional NameMaps are also made available
 * @param ppSimulationState is a pointer to the handle to fill with the newly created simulation
 * when successful
 * @return FOE_SIMULATION_SUCCESS on a successful creation, an appropriate error code otherwise.
 *
 * The 'pCreateFn' of any previously registered functionality is called on the created simulation.
 */
FOE_SIM_EXPORT foeResult foeCreateSimulation(bool addNameMaps, foeSimulation **ppSimulationState);

/**
 * @brief Attempts to destroy a given SimulationState
 * @param pSimulation Object to attempt to destroy
 * @return FOE_SIMULATION_SUCCESS if successfully destroyed. A descriptive error code otherwise.
 *
 * The given SimulationState is first checked to see if it was created by the given registry and if
 * not, returns false without interacting with it.
 *
 * If the simulation is valid and was previously initialized, the 'onDeinitialize' is called.
 * 'pDestroyFn' is called last and the object is then freed.
 */
FOE_SIM_EXPORT foeResult foeDestroySimulation(foeSimulation *pSimulation);

/**
 * @brief Initializes a SimulationState given InitInfo
 * @param pSimulation SimulationState to initialize
 * @param pInitInfo Required information used to initialize a SimulationState
 * @param FOE_SIMULATION_SUCCESS if successfully destroyed. A descriptive error code otherwise.
 *
 * Returns immediately if the SimulationState had already been initialized.
 *
 * Otherwise iterates through any registered functionality and calls its 'pInitializeFn'
 * function. If any of these fails, then it backtracks deinitializing anything that did succeed and
 * returns a state fully deinitialized.
 */
FOE_SIM_EXPORT foeResult foeInitializeSimulation(foeSimulation *pSimulation,
                                                 foeSimulationInitInfo const *pInitInfo);

/**
 * @brief Deinitializes a SimulationState given InitInfo
 * @param pSimulation SimulationState to deinitialize
 * @return FOE_SIMULATION_SUCCESS if deinitialization proceeded, otherwise
 * FOE_SIMULATION_ERROR_SIMULATION_NOT_INITIALIZED if the simulation has not previously been
 * initialized.
 *
 * Iterates through any registered functionality and calls its 'pDeinitializeFn' function.
 */
FOE_SIM_EXPORT foeResult foeDeinitializeSimulation(foeSimulation *pSimulation);

/**
 * @brief Initializes graphics for a SimulationState
 * @param pSimulation is a pointer to the simulation that graphics is being initialized for
 * @param gfxSession is a handle to the graphics session the simulation will be using
 * @return An appropriate error code is returned. FOE_SIMULATION_SUCCESS if the simulation
 * initialized using the graphics session successfully.
 * FOE_SIMULATION_ERROR_SIMULATION_GRAPHICS_ALREADY_INITIALIZED if the simulation already was
 * initialized successfully with graphics previously. Otherwise another detailed error_code from a
 * failed attempt to initialize some functionality will be returned.
 *
 */
FOE_SIM_EXPORT foeResult foeInitializeSimulationGraphics(foeSimulation *pSimulation,
                                                         foeGfxSession gfxSession);

/**
 * @brief Deinitializes graphics from the given simulation
 * @param pSimulation is a pointer to the simulation to deinitialize graphics from
 * @return An appropriate error code is returned. FOE_SIMULATION_SUCCESS if deinitialization
 * happened without issue. FOE_SIMULATION_ERROR_SIMULATION_GRAPHICS_NOT_INITIALIZED if the
 * simulation's graphics was not previously successfully initialized.
 */
FOE_SIM_EXPORT foeResult foeDeinitializeSimulationGraphics(foeSimulation *pSimulation);

FOE_SIM_EXPORT foeResult foeSimulationGetRefCount(foeSimulation const *pSimulation,
                                                  foeSimulationStructureType sType,
                                                  size_t *pRefCount);

FOE_SIM_EXPORT foeResult foeSimulationIncrementRefCount(foeSimulation *pSimulation,
                                                        foeSimulationStructureType sType,
                                                        size_t *pRefCount);

FOE_SIM_EXPORT foeResult foeSimulationDecrementRefCount(foeSimulation *pSimulation,
                                                        foeSimulationStructureType sType,
                                                        size_t *pRefCount);

FOE_SIM_EXPORT foeResult foeSimulationGetInitCount(foeSimulation const *pSimulation,
                                                   foeSimulationStructureType sType,
                                                   size_t *pInitCount);

FOE_SIM_EXPORT foeResult foeSimulationIncrementInitCount(foeSimulation *pSimulation,
                                                         foeSimulationStructureType sType,
                                                         size_t *pInitCount);

FOE_SIM_EXPORT foeResult foeSimulationDecrementInitCount(foeSimulation *pSimulation,
                                                         foeSimulationStructureType sType,
                                                         size_t *pInitCount);

FOE_SIM_EXPORT foeResult foeSimulationGetGfxInitCount(foeSimulation const *pSimulation,
                                                      foeSimulationStructureType sType,
                                                      size_t *pGfxInitCount);

FOE_SIM_EXPORT foeResult foeSimulationIncrementGfxInitCount(foeSimulation *pSimulation,
                                                            foeSimulationStructureType sType,
                                                            size_t *pGfxInitCount);

FOE_SIM_EXPORT foeResult foeSimulationDecrementGfxInitCount(foeSimulation *pSimulation,
                                                            foeSimulationStructureType sType,
                                                            size_t *pGfxInitCount);

FOE_SIM_EXPORT foeResult foeSimulationInsertResourceLoader(
    foeSimulation *pSimulation, foeSimulationLoaderData const *pCreateInfo);

FOE_SIM_EXPORT foeResult foeSimulationReleaseResourceLoader(foeSimulation *pSimulation,
                                                            foeSimulationStructureType sType,
                                                            void **ppLoader);

FOE_SIM_EXPORT foeResult foeSimulationInsertComponentPool(
    foeSimulation *pSimulation, foeSimulationComponentPoolData const *pCreateInfo);

FOE_SIM_EXPORT foeResult foeSimulationReleaseComponentPool(foeSimulation *pSimulation,
                                                           foeSimulationStructureType sType,
                                                           void **ppComponentPool);

FOE_SIM_EXPORT foeResult foeSimulationInsertSystem(foeSimulation *pSimulation,
                                                   foeSimulationSystemData const *pCreateInfo);

FOE_SIM_EXPORT foeResult foeSimulationReleaseSystem(foeSimulation *pSimulation,
                                                    foeSimulationStructureType sType,
                                                    void **ppSystem);

FOE_SIM_EXPORT foeResourceCreateInfo
TEMP_foeSimulationGetResourceCreateInfo(void *pContext, foeResourceID resourceID);

FOE_SIM_EXPORT void TEMP_foeSimulationLoadResource(void *pContext,
                                                   foeResource resource,
                                                   PFN_foeResourcePostLoad *pPostLoadFn);

#endif // FOE_SIMULATION_CORE_HPP