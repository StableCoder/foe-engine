// Copyright (C) 2021-2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_SIMULATION_SIMULATION_H
#define FOE_SIMULATION_SIMULATION_H

#include <foe/ecs/id.h>
#include <foe/ecs/name_map.h>
#include <foe/graphics/session.h>
#include <foe/resource/pool.h>
#include <foe/resource/resource.h>
#include <foe/simulation/export.h>
#include <foe/simulation/group_data.h>
#include <foe/simulation/resource_create_info_history.h>
#include <foe/simulation/resource_create_info_pool.h>

#ifdef __cplusplus
extern "C" {
#endif

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

FOE_DEFINE_HANDLE(foeSimulation)

typedef int foeSimulationStructureType;

struct foeSimulationBaseStruct {
    foeSimulationStructureType sType;
    void *pNext;
};

struct foeResourceLoaderBase;
struct foeComponentPoolBase;
struct foeSystemBase;

typedef foeResultSet (*PFN_foeSimulationExternalFileSearch)(void *,
                                                            char const *,
                                                            foeManagedMemory *);

typedef struct foeSimulationInitInfo {
    void *pExternalFileSearchContext;
    PFN_foeSimulationExternalFileSearch pfnExternalFileSearch;
} foeSimulationInitInfo;

typedef struct foeSimulationLoaderData {
    foeSimulationStructureType sType;
    size_t refCount;
    size_t initCount;
    size_t gfxInitCount;
    /// The loader itself
    void *pLoader;
    /// Function that returns whether the loader can process a given ResourceCreateInfo type
    bool (*pCanProcessCreateInfoFn)(foeResourceCreateInfo);
    /// Function to be called to load a resource with a compatible ResourceCreateInfo
    void (*pLoadFn)(void *, foeResource, foeResourceCreateInfo, PFN_foeResourcePostLoad);
    /// Maintenance to be performed as part of the regular simulation loop
    void (*pMaintenanceFn)(void *);
    /// Maintenance to be performed as part of the graphics loop
    void (*pGfxMaintenanceFn)(void *);
} foeSimulationLoaderData;

typedef struct foeSimulationComponentPoolData {
    foeSimulationStructureType sType;
    size_t refCount;
    void *pComponentPool;
    void (*pMaintenanceFn)(void *);
} foeSimulationComponentPoolData;

typedef struct foeSimulationSystemData {
    foeSimulationStructureType sType;
    size_t refCount;
    size_t initCount;
    size_t gfxInitCount;
    void *pSystem;
} foeSimulationSystemData;

/// Return if a simulation has been successfully initialized
bool foeSimulationIsInitialized(foeSimulation simulation);

/// Returns if a simulation has had graphics successfully initialized
bool foeSimulationIsGraphicsInitialzied(foeSimulation simulation);

/**
 * @brief Creates a new SimulationState with any registered functionality available
 * @param addNameMaps If true, the optional NameMaps are also made available
 * @param ppSimulationState is a pointer to the handle to fill with the newly created simulation
 * when successful
 * @return FOE_SIMULATION_SUCCESS on a successful creation, an appropriate error code otherwise.
 *
 * The 'pCreateFn' of any previously registered functionality is called on the created simulation.
 */
FOE_SIM_EXPORT
foeResultSet foeCreateSimulation(bool addNameMaps, foeSimulation *pSimulation);

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
FOE_SIM_EXPORT
foeResultSet foeDestroySimulation(foeSimulation simulation);

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
FOE_SIM_EXPORT
foeResultSet foeInitializeSimulation(foeSimulation simulation,
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
FOE_SIM_EXPORT
foeResultSet foeDeinitializeSimulation(foeSimulation simulation);

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
FOE_SIM_EXPORT
foeResultSet foeInitializeSimulationGraphics(foeSimulation simulation, foeGfxSession gfxSession);

/**
 * @brief Deinitializes graphics from the given simulation
 * @param pSimulation is a pointer to the simulation to deinitialize graphics from
 * @return An appropriate error code is returned. FOE_SIMULATION_SUCCESS if deinitialization
 * happened without issue. FOE_SIMULATION_ERROR_SIMULATION_GRAPHICS_NOT_INITIALIZED if the
 * simulation's graphics was not previously successfully initialized.
 */
FOE_SIM_EXPORT
foeResultSet foeDeinitializeSimulationGraphics(foeSimulation simulation);

FOE_SIM_EXPORT
foeResultSet foeSimulationGetRefCount(foeSimulation simulation,
                                      foeSimulationStructureType sType,
                                      size_t *pRefCount);

FOE_SIM_EXPORT
foeResultSet foeSimulationIncrementRefCount(foeSimulation simulation,
                                            foeSimulationStructureType sType,
                                            size_t *pRefCount);

FOE_SIM_EXPORT
foeResultSet foeSimulationDecrementRefCount(foeSimulation simulation,
                                            foeSimulationStructureType sType,
                                            size_t *pRefCount);

FOE_SIM_EXPORT
foeResultSet foeSimulationGetInitCount(foeSimulation simulation,
                                       foeSimulationStructureType sType,
                                       size_t *pInitCount);

FOE_SIM_EXPORT
foeResultSet foeSimulationIncrementInitCount(foeSimulation simulation,
                                             foeSimulationStructureType sType,
                                             size_t *pInitCount);

FOE_SIM_EXPORT
foeResultSet foeSimulationDecrementInitCount(foeSimulation simulation,
                                             foeSimulationStructureType sType,
                                             size_t *pInitCount);

FOE_SIM_EXPORT
foeResultSet foeSimulationGetGfxInitCount(foeSimulation simulation,
                                          foeSimulationStructureType sType,
                                          size_t *pGfxInitCount);

FOE_SIM_EXPORT
foeResultSet foeSimulationIncrementGfxInitCount(foeSimulation simulation,
                                                foeSimulationStructureType sType,
                                                size_t *pGfxInitCount);

FOE_SIM_EXPORT
foeResultSet foeSimulationDecrementGfxInitCount(foeSimulation simulation,
                                                foeSimulationStructureType sType,
                                                size_t *pGfxInitCount);

FOE_SIM_EXPORT
foeResultSet foeSimulationInsertResourceLoader(foeSimulation simulation,
                                               foeSimulationLoaderData const *pCreateInfo);

FOE_SIM_EXPORT
foeResultSet foeSimulationReleaseResourceLoader(foeSimulation simulation,
                                                foeSimulationStructureType sType,
                                                void **ppLoader);

FOE_SIM_EXPORT
foeResultSet foeSimulationInsertComponentPool(foeSimulation simulation,
                                              foeSimulationComponentPoolData const *pCreateInfo);

FOE_SIM_EXPORT
foeResultSet foeSimulationReleaseComponentPool(foeSimulation simulation,
                                               foeSimulationStructureType sType,
                                               void **ppComponentPool);

FOE_SIM_EXPORT
foeResultSet foeSimulationInsertSystem(foeSimulation simulation,
                                       foeSimulationSystemData const *pCreateInfo);

FOE_SIM_EXPORT
foeResultSet foeSimulationReleaseSystem(foeSimulation simulation,
                                        foeSimulationStructureType sType,
                                        void **ppSystem);

FOE_SIM_EXPORT
foeResultSet foeSimulationGetResourceCreateInfo(foeSimulation simulation,
                                                foeResourceID resourceID,
                                                foeResourceCreateInfo *pResourceCI);

FOE_SIM_EXPORT foeResourcePool foeSimulationGetResourcePool(foeSimulation simulation);

FOE_SIM_EXPORT foeGroupData foeSimulationGetGroupData(foeSimulation simulation);

FOE_SIM_EXPORT foeEcsNameMap foeSimulationGetEntityNameMap(foeSimulation simulation);

FOE_SIM_EXPORT foeEcsNameMap foeSimulationGetResourceNameMap(foeSimulation simulation);

FOE_SIM_EXPORT void foeSimulationGetResourceLoaders(foeSimulation simulation,
                                                    size_t *pCount,
                                                    foeSimulationLoaderData **ppLoaders);

FOE_SIM_EXPORT void foeSimulationGetComponentPools(
    foeSimulation simulation, size_t *pCount, foeSimulationComponentPoolData **ppComponentPools);

FOE_SIM_EXPORT foeResourceCreateInfoPool
foeSimulationGetSavedBaseDataResourceCreateInfoPool(foeSimulation simulation);

FOE_SIM_EXPORT foeResourceCreateInfoPool
foeSimulationGetSavedPersistentDataResourceCreateInfoPool(foeSimulation simulation);

FOE_SIM_EXPORT foeResourceCreateInfoHistory
foeSimulationGetSessionPersistentDataResourceCreateInfoHistory(foeSimulation simulation);

FOE_SIM_EXPORT
void *foeSimulationGetResourceLoader(foeSimulation simulation, foeSimulationStructureType sType);

FOE_SIM_EXPORT
void *foeSimulationGetSystem(foeSimulation simulation, foeSimulationStructureType sType);

FOE_SIM_EXPORT
void *foeSimulationGetComponentPool(foeSimulation simulation, foeSimulationStructureType sType);

#ifdef __cplusplus
}
#endif

#endif // FOE_SIMULATION_SIMULATION_H