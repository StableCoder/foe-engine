// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/simulation/registration.hpp>
#include <foe/simulation/simulation.hpp>

#include <foe/chrono/easy_clock.hpp>
#include <foe/ecs/name_map.h>
#include <foe/resource/resource_fns.h>

#include "log.hpp"
#include "result.h"

#include <mutex>
#include <vector>

namespace {
std::mutex mSync;

std::vector<foeSimulationFunctionalty> mRegistered;
std::vector<foeSimulation *> mStates;

void acquireExclusiveLock(foeSimulation *pSimulation, char const *pReason) {
    FOE_LOG(foeSimulation, FOE_LOG_LEVEL_VERBOSE,
            "[{}] foeSimulation - Acquiring exclusive lock for {}",
            static_cast<void *>(pSimulation), pReason)
    foeEasyHighResClock waitingTime;

    pSimulation->simSync.lock();

    waitingTime.update();
    FOE_LOG(foeSimulation, FOE_LOG_LEVEL_VERBOSE,
            "[{}] foeSimulation - Acquired exclusive lock for {} after {}ms",
            static_cast<void *>(pSimulation), pReason,
            waitingTime.elapsed<std::chrono::milliseconds>().count())
}

// Assumes that the mutex has already been acquired
void deinitSimulation(foeSimulation *pSimulation) {
    FOE_LOG(foeSimulation, FOE_LOG_LEVEL_VERBOSE, "[{}] foeSimulation - Deinitializing",
            static_cast<void *>(pSimulation));

    // Deinit functionality
    auto const endIt = mRegistered.rend();
    for (auto it = mRegistered.rbegin(); it != endIt; ++it) {
        if (it->pDeinitializeFn) {
            it->pDeinitializeFn(pSimulation);
        }
    }

    pSimulation->initInfo = {};

    pSimulation->simSync.unlock();

    FOE_LOG(foeSimulation, FOE_LOG_LEVEL_VERBOSE, "[{}] foeSimulation - Deinitialized",
            static_cast<void *>(pSimulation));
}

// Assumes that the mutex has already been acquired
void deinitializeSimulationGraphics(foeSimulation *pSimulation) {
    FOE_LOG(foeSimulation, FOE_LOG_LEVEL_VERBOSE, "[{}] foeSimulation - Deinitializing Graphics",
            static_cast<void *>(pSimulation));

    // Iterate through all and deinitialize
    auto const endIt = mRegistered.rend();
    for (auto it = mRegistered.rbegin(); it != endIt; ++it) {
        if (it->pDeinitializeGraphicsFn) {
            it->pDeinitializeGraphicsFn(pSimulation);
        }
    }

    pSimulation->gfxSession = FOE_NULL_HANDLE;

    pSimulation->simSync.unlock();

    FOE_LOG(foeSimulation, FOE_LOG_LEVEL_VERBOSE, "[{}] foeSimulation - Deinitialized Graphics",
            static_cast<void *>(pSimulation));
}

foeResourceCreateInfo getResourceCreateInfo(void *pContext, foeResourceID resourceID) {
    auto *pSimulation = reinterpret_cast<foeSimulation *>(pContext);

    foeIdGroup group = foeIdGetGroup(resourceID);
    if (group == foeIdPersistentGroup) {
        foeResourceCreateInfo resourceCI = FOE_NULL_HANDLE;
        resourceCI = foeResourceCreateInfoHistoryCurrent(
            pSimulation->resourceCreateInfoSessionPersistentData, resourceID);
        if (resourceCI != FOE_NULL_HANDLE)
            return resourceCI;

        resourceCI = foeResourceCreateInfoPoolGet(
            pSimulation->resourceCreateInfoSavedPersistentData, resourceID);
        if (resourceCI != FOE_NULL_HANDLE)
            return resourceCI;
    } else if (group < foeIdPersistentGroup) {
        foeResourceCreateInfo resourceCI = FOE_NULL_HANDLE;
        resourceCI =
            foeResourceCreateInfoPoolGet(pSimulation->resourceCreateInfoSavedBaseData, resourceID);
        if (resourceCI != FOE_NULL_HANDLE)
            return resourceCI;
    }

    return FOE_NULL_HANDLE;
}

void loadResource(void *pContext, foeResource resource, PFN_foeResourcePostLoad postLoadFn) {
    auto *pSimulation = reinterpret_cast<foeSimulation *>(pContext);

    foeResourceCreateInfo resourceCreateInfo =
        getResourceCreateInfo(pSimulation, foeResourceGetID(resource));

    if (resourceCreateInfo == FOE_NULL_HANDLE) {
        postLoadFn(resource, to_foeResult(FOE_SIMULATION_ERROR_NO_CREATE_INFO), nullptr, nullptr,
                   nullptr, nullptr);
        return;
    }

    for (auto const &it : pSimulation->resourceLoaders) {
        if (it.pCanProcessCreateInfoFn(resourceCreateInfo)) {
            it.pLoadFn(it.pLoader, resource, resourceCreateInfo, postLoadFn);
            return;
        }
    }

    // If no suitable loader could be found
    postLoadFn(resource, to_foeResult(FOE_SIMULATION_ERROR_NO_LOADER_FOUND), nullptr, nullptr,
               nullptr, nullptr);
    foeResourceCreateInfoDecrementRefCount(resourceCreateInfo);
}

} // namespace

foeResultSet foeRegisterFunctionality(foeSimulationFunctionalty const &functionality) {
    std::scoped_lock lock{mSync};

    if (functionality.id != 0 && (functionality.id < 1000000000 || functionality.id % 1000 != 0)) {
        FOE_LOG(foeSimulation, FOE_LOG_LEVEL_WARNING,
                "foeRegisterFunctionality - Attempted to register functionality with invalid ID");
        return to_foeResult(FOE_SIMULATION_ERROR_ID_INVALID);
    }

    for (auto const &it : mRegistered) {
        if (it.id == functionality.id) {
            FOE_LOG(foeSimulation, FOE_LOG_LEVEL_WARNING,
                    "foeRegisterFunctionality - Attempted to register functionality with an ID "
                    "already in use");
            return to_foeResult(FOE_SIMULATION_ERROR_ID_ALREADY_IN_USE);
        }
    }

    // Not already registered, add it.
    mRegistered.emplace_back(functionality);

    // Go through any already existing foeSimulation's and add this new functionality to them.
    struct {
        bool created;
        bool initialized;
        bool gfxInitialized;
    } passedStates;

    foeResultSet result = to_foeResult(FOE_SIMULATION_SUCCESS);
    foeSimulation **ppSimState = mStates.data();
    foeSimulation **ppEndSimState = mStates.data() + mStates.size();

    for (; ppSimState != ppEndSimState; ++ppSimState) {
        // Reset the set of passed calls
        passedStates = {};

        acquireExclusiveLock(*ppSimState, "functionality registration");

        if (functionality.pCreateFn) {
            result = functionality.pCreateFn(*ppSimState);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(foeSimulation, FOE_LOG_LEVEL_ERROR,
                        "foeRegisterFunctionality - Failed creating functionality on "
                        "foeSimulation: {} due "
                        "to error: {}",
                        static_cast<void *>(*ppSimState), buffer);
                break;
            }
            passedStates.created = true;
        }
        if (foeSimulationIsInitialized(*ppSimState) && functionality.pInitializeFn) {
            result = functionality.pInitializeFn(*ppSimState, &(*ppSimState)->initInfo);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(foeSimulation, FOE_LOG_LEVEL_ERROR,
                        "foeRegisterFunctionality - Failed initializing functionality on "
                        "foeSimulation: {} due to error: {}",
                        static_cast<void *>(*ppSimState), buffer);
                break;
            }
            passedStates.initialized = true;
        }
        if (foeSimulationIsGraphicsInitialzied(*ppSimState) &&
            functionality.pInitializeGraphicsFn) {
            result = functionality.pInitializeGraphicsFn(*ppSimState, (*ppSimState)->gfxSession);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(foeSimulation, FOE_LOG_LEVEL_ERROR,
                        "foeRegisterFunctionality - Failed initializing graphics functionality on "
                        "foeSimulation: {} due to error: {}",
                        static_cast<void *>(*ppSimState), buffer);
                break;
            }
            passedStates.gfxInitialized = true;
        }

        (*ppSimState)->simSync.unlock();
        if (result.value != FOE_SUCCESS)
            break;
    }

    // If we failed to initialize in one of the simulations above, then we need to go through and
    // deinitialize/destroy this functionality in all the ones it was already added to successfully
    if (result.value != FOE_SUCCESS && ppSimState != mStates.data()) {
        // Deal with the potential half-initialized simState that was being worked on
        if (passedStates.gfxInitialized && functionality.pDeinitializeGraphicsFn &&
            foeSimulationIsGraphicsInitialzied(*ppSimState)) {
            functionality.pDeinitializeGraphicsFn(*ppSimState);
        }
        if (passedStates.initialized && functionality.pDeinitializeFn &&
            foeSimulationIsInitialized(*ppSimState)) {
            functionality.pDeinitializeFn(*ppSimState);
        }
        if (passedStates.created && functionality.pDestroyFn) {
            functionality.pDeinitializeFn(*ppSimState);
        }

        (*ppSimState)->simSync.unlock();

        // Go through the rest of the simulations
        --ppSimState;
        ppEndSimState = mStates.data() - 1;
        for (; ppSimState != ppEndSimState; --ppSimState) {
            acquireExclusiveLock(*ppSimState, "functionality deregistration");

            if (functionality.pDeinitializeGraphicsFn &&
                foeSimulationIsGraphicsInitialzied(*ppSimState)) {
                functionality.pDeinitializeGraphicsFn(*ppSimState);
            }

            if (functionality.pDeinitializeFn && foeSimulationIsInitialized(*ppSimState)) {
                functionality.pDeinitializeFn(*ppSimState);
            }

            if (functionality.pDestroyFn) {
                functionality.pDestroyFn(*ppSimState);
            }
            (*ppSimState)->simSync.unlock();
        }

        return result;
    }

    return to_foeResult(FOE_SIMULATION_SUCCESS);
}

foeResultSet foeDeregisterFunctionality(foeSimulationUUID functionalityUUID) {
    std::scoped_lock lock{mSync};

    for (auto it = mRegistered.begin(); it != mRegistered.end(); ++it) {
        if (it->id == functionalityUUID) {
            // Since we're deregistering functionality, we need to deinit/destroy this stuff from
            // any active SimulationStates
            for (auto *pSimState : mStates) {
                acquireExclusiveLock(pSimState, "functionality deregistration");
                if (it->pDeinitializeGraphicsFn && foeSimulationIsGraphicsInitialzied(pSimState)) {
                    it->pDeinitializeGraphicsFn(pSimState);
                }

                if (it->pDeinitializeFn && foeSimulationIsInitialized(pSimState)) {
                    it->pDeinitializeFn(pSimState);
                }

                if (it->pDestroyFn) {
                    it->pDestroyFn(pSimState);
                }
                pSimState->simSync.unlock();
            }

            mRegistered.erase(it);
            return to_foeResult(FOE_SIMULATION_SUCCESS);
        }
    }

    FOE_LOG(
        foeSimulation, FOE_LOG_LEVEL_WARNING,
        "registerFunctionality - Attempted to deregister functionality that was never registered");
    return to_foeResult(FOE_SIMULATION_ERROR_NOT_REGISTERED);
}

bool foeSimulationIsInitialized(foeSimulation const *pSimulation) {
    return !!pSimulation->initInfo.externalFileSearchFn;
}

bool foeSimulationIsGraphicsInitialzied(foeSimulation const *pSimulation) {
    return pSimulation->gfxSession != FOE_NULL_HANDLE;
}

foeResultSet foeCreateSimulation(bool addNameMaps, foeSimulation **ppSimulationState) {
    foeResultSet result;
    std::scoped_lock lock{mSync};

    std::unique_ptr<foeSimulation> newSimState{new (std::nothrow) foeSimulation};
    if (newSimState == nullptr)
        return to_foeResult(FOE_SIMULATION_ERROR_OUT_OF_MEMORY);

    newSimState->gfxSession = FOE_NULL_HANDLE;
    newSimState->resourceCreateInfoSavedBaseData = FOE_NULL_HANDLE;
    newSimState->resourceCreateInfoSavedPersistentData = FOE_NULL_HANDLE;
    newSimState->resourceCreateInfoSessionPersistentData = FOE_NULL_HANDLE;
    newSimState->resourcePool = FOE_NULL_HANDLE;

    // Saved Base Data
    result = foeCreateResourceCreateInfoPool(&newSimState->resourceCreateInfoSavedBaseData);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeSimulation, FOE_LOG_LEVEL_ERROR,
                "foeSimulation - Failed to create foeResourceCreateInfoPool (Base) due to: {}",
                buffer);

        return result;
    }

    // Saved Persistent Data
    result = foeCreateResourceCreateInfoPool(&newSimState->resourceCreateInfoSavedPersistentData);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(
            foeSimulation, FOE_LOG_LEVEL_ERROR,
            "foeSimulation - Failed to create foeResourceCreateInfoPool (Persistent) due to: {}",
            buffer);

        return result;
    }

    // Session Persistent Data
    result =
        foeCreateResourceCreateInfoHistory(&newSimState->resourceCreateInfoSessionPersistentData);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeSimulation, FOE_LOG_LEVEL_ERROR,
                "foeSimulation - Failed to create foeResourceCreateInfoHistory due to: {}", buffer);

        return result;
    }

    // Resource Pool
    foeResourceFns resourceCallbacks{
        .pLoadContext = newSimState.get(),
        .pLoadFn = loadResource,
    };

    result = foeCreateResourcePool(&resourceCallbacks, &newSimState->resourcePool);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeSimulation, FOE_LOG_LEVEL_ERROR,
                "foeSimulation - Failed to create ResourcePool due to: {}", buffer);

        return result;
    }

    // Editor Name Maps, if requested
    if (addNameMaps) {
        result = foeEcsCreateNameMap(&newSimState->resourceNameMap);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeSimulation, FOE_LOG_LEVEL_ERROR,
                    "foeSimulation - Failed to create Name Map due to: {}", buffer);

            return result;
        }

        result = foeEcsCreateNameMap(&newSimState->entityNameMap);
        if (result.value != FOE_SUCCESS) {
            foeEcsDestroyNameMap(newSimState->resourceNameMap);

            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeSimulation, FOE_LOG_LEVEL_ERROR,
                    "foeSimulation - Failed to create Name Map due to: {}", buffer);

            return result;
        }
    }

    FOE_LOG(foeSimulation, FOE_LOG_LEVEL_VERBOSE, "[{}] foeSimulation - Creating",
            static_cast<void *>(newSimState.get()));

    // Go through each registered set of functionality, add its items
    auto it = mRegistered.begin();
    auto const endIt = mRegistered.end();
    for (; it != endIt; ++it) {
        if (it->pCreateFn) {
            result = it->pCreateFn(newSimState.get());
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(foeSimulation, FOE_LOG_LEVEL_ERROR,
                        "[{}] foeSimulation - Error creating due to error: {}",
                        static_cast<void *>(newSimState.get()), buffer);

                break;
            }
        }
    }

    // If there is an error code at this point, then we need to walk-back what was created so far
    if (result.value != FOE_SUCCESS && it != mRegistered.begin()) {
        --it;
        for (; it >= mRegistered.begin(); --it) {
            it->pDestroyFn(newSimState.get());
        }
    }

    if (result.value == FOE_SUCCESS) {
        FOE_LOG(foeSimulation, FOE_LOG_LEVEL_VERBOSE, "[{}] foeSimulation - Created",
                static_cast<void *>(newSimState.get()));
        mStates.emplace_back(newSimState.get());
        *ppSimulationState = newSimState.release();
    } else {
        foeDestroySimulation(newSimState.get());
    }

    return result;
}

foeResultSet foeDestroySimulation(foeSimulation *pSimulation) {
    std::scoped_lock lock{mSync};

    FOE_LOG(foeSimulation, FOE_LOG_LEVEL_VERBOSE, "[{}] foeSimulation - Destroying",
            static_cast<void *>(pSimulation));

    auto searchIt = std::find(mStates.begin(), mStates.end(), pSimulation);
    if (searchIt == mStates.end()) {
        // We were given a simulation state that wasn't created from here?
        FOE_LOG(foeSimulation, FOE_LOG_LEVEL_WARNING,
                "[{}] foeSimulation - Given a foeSimulation that wasn't created via "
                "foeCreateSimulation and isn't registered",
                static_cast<void *>(pSimulation));
        return to_foeResult(FOE_SIMULATION_ERROR_NOT_REGISTERED);
    } else {
        mStates.erase(searchIt);
    }

    acquireExclusiveLock(pSimulation, "simulation destruction");

    if (foeSimulationIsInitialized(pSimulation))
        deinitSimulation(pSimulation);

    auto const endIt = mRegistered.rend();
    for (auto it = mRegistered.rbegin(); it != endIt; ++it) {
        if (it->pDestroyFn) {
            it->pDestroyFn(pSimulation);
        }
    }
    pSimulation->simSync.unlock();

    // Destroy Name Maps
    if (pSimulation->entityNameMap != FOE_NULL_HANDLE)
        foeEcsDestroyNameMap(pSimulation->entityNameMap);
    if (pSimulation->resourceNameMap != FOE_NULL_HANDLE)
        foeEcsDestroyNameMap(pSimulation->resourceNameMap);

    // Destroy ResourcePool
    foeDestroyResourcePool(pSimulation->resourcePool);

    // Destroy Records
    if (pSimulation->resourceCreateInfoSessionPersistentData != FOE_NULL_HANDLE)
        foeDestroyResourceCreateInfoHistory(pSimulation->resourceCreateInfoSessionPersistentData);
    if (pSimulation->resourceCreateInfoSavedPersistentData != FOE_NULL_HANDLE)
        foeDestroyResourceCreateInfoPool(pSimulation->resourceCreateInfoSavedPersistentData);
    if (pSimulation->resourceCreateInfoSavedBaseData != FOE_NULL_HANDLE)
        foeDestroyResourceCreateInfoPool(pSimulation->resourceCreateInfoSavedBaseData);

    // Delete it
    delete pSimulation;

    FOE_LOG(foeSimulation, FOE_LOG_LEVEL_VERBOSE, "[{}] foeSimulation - Destroyed",
            static_cast<void *>(pSimulation));

    return to_foeResult(FOE_SIMULATION_SUCCESS);
}

foeResultSet foeInitializeSimulation(foeSimulation *pSimulation,
                                     foeSimulationInitInfo const *pInitInfo) {
    std::scoped_lock lock{mSync};

    if (foeSimulationIsInitialized(pSimulation)) {
        FOE_LOG(foeSimulation, FOE_LOG_LEVEL_ERROR,
                "[{}] foeSimulation - Attempted to re-initialize", static_cast<void *>(pSimulation))
        return to_foeResult(FOE_SIMULATION_ERROR_SIMULATION_ALREADY_INITIALIZED);
    }

    FOE_LOG(foeSimulation, FOE_LOG_LEVEL_VERBOSE, "[{}] foeSimulation - Initializing",
            static_cast<void *>(pSimulation));

    acquireExclusiveLock(pSimulation, "initialization");

    // Go through each set of functionality and call the initialization function, if one is attached
    foeResultSet result = to_foeResult(FOE_SIMULATION_SUCCESS);
    auto it = mRegistered.begin();
    auto const endIt = mRegistered.end();

    for (; it != endIt; ++it) {
        if (it->pInitializeFn) {
            result = it->pInitializeFn(pSimulation, pInitInfo);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(foeSimulation, FOE_LOG_LEVEL_ERROR,
                        "[{}] foeSimulation - Failed to initialize due to error: {}",
                        static_cast<void *>(pSimulation), buffer);
                break;
            }
        }
    }

    // If there is an error code at this point, then we need to walk-back what was created so far
    if (result.value != FOE_SUCCESS && it != mRegistered.begin()) {
        --it;
        for (; it >= mRegistered.begin(); --it) {
            if (it->pDeinitializeFn) {
                it->pDeinitializeFn(pSimulation);
            }
        }
    }

    if (result.value == FOE_SUCCESS) {
        // On a successful initialization, set it into the state itself
        pSimulation->initInfo = *pInitInfo;
        FOE_LOG(foeSimulation, FOE_LOG_LEVEL_VERBOSE, "[{}] foeSimulation - Initialized",
                static_cast<void *>(pSimulation));
    }

    pSimulation->simSync.unlock();

    return result;
}

foeResultSet foeDeinitializeSimulation(foeSimulation *pSimulation) {
    std::scoped_lock lock{mSync};

    if (!foeSimulationIsInitialized(pSimulation))
        return to_foeResult(FOE_SIMULATION_ERROR_SIMULATION_NOT_INITIALIZED);

    acquireExclusiveLock(pSimulation, "deinitialization");
    deinitSimulation(pSimulation);

    return to_foeResult(FOE_SIMULATION_SUCCESS);
}

foeResultSet foeInitializeSimulationGraphics(foeSimulation *pSimulation, foeGfxSession gfxSession) {
    std::scoped_lock lock{mSync};

    if (foeSimulationIsGraphicsInitialzied(pSimulation)) {
        FOE_LOG(foeSimulation, FOE_LOG_LEVEL_ERROR,
                "[{}] foeSimulation - Attempted to re-initialize graphics",
                static_cast<void *>(pSimulation))
        return to_foeResult(FOE_SIMULATION_ERROR_SIMULATION_GRAPHICS_ALREADY_INITIALIZED);
    }

    FOE_LOG(foeSimulation, FOE_LOG_LEVEL_VERBOSE, "[{}] foeSimulation - Initializing graphics",
            static_cast<void *>(pSimulation));

    acquireExclusiveLock(pSimulation, "initializing graphics");

    // Go through each set of functionality and call the initialization function, if one is attached
    foeResultSet result = to_foeResult(FOE_SIMULATION_SUCCESS);
    auto it = mRegistered.begin();
    auto const endIt = mRegistered.end();

    for (; it != endIt; ++it) {
        if (it->pInitializeGraphicsFn) {
            result = it->pInitializeGraphicsFn(pSimulation, gfxSession);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(foeSimulation, FOE_LOG_LEVEL_ERROR,
                        "[{}] foeSimulation - Failed to initialize graphics due to error: {}",
                        static_cast<void *>(pSimulation), buffer);

                break;
            }
        }
    }

    // If there is an error code at this point, then we need to walk-back what was created so far
    if (result.value != FOE_SUCCESS && it != mRegistered.begin()) {
        --it;
        for (; it >= mRegistered.begin(); --it) {
            if (it->pDeinitializeGraphicsFn) {
                it->pDeinitializeGraphicsFn(pSimulation);
            }
        }
    }

    if (result.value == FOE_SUCCESS) {
        // With success, set the simulation's graphics session handle
        pSimulation->gfxSession = gfxSession;
        FOE_LOG(foeSimulation, FOE_LOG_LEVEL_VERBOSE, "[{}] foeSimulation - Initialized graphics",
                static_cast<void *>(pSimulation));
    }

    pSimulation->simSync.unlock();

    return result;
}

foeResultSet foeDeinitializeSimulationGraphics(foeSimulation *pSimulation) {
    std::scoped_lock lock{mSync};

    if (!foeSimulationIsGraphicsInitialzied(pSimulation)) {
        FOE_LOG(foeSimulation, FOE_LOG_LEVEL_WARNING,
                "[{}] foeSimulation - Attempted to deinitialize uninitialized graphics");
        return to_foeResult(FOE_SIMULATION_ERROR_SIMULATION_GRAPHICS_NOT_INITIALIZED);
    }

    acquireExclusiveLock(pSimulation, "deinitializing graphics");
    deinitializeSimulationGraphics(pSimulation);
    return to_foeResult(FOE_SIMULATION_SUCCESS);
}

foeResultSet foeSimulationGetRefCount(foeSimulation const *pSimulation,
                                      foeSimulationStructureType sType,
                                      size_t *pRefCount) {
    // Resource Loaders
    for (auto const &it : pSimulation->resourceLoaders) {
        if (it.sType == sType) {
            *pRefCount = it.refCount;

            return to_foeResult(FOE_SIMULATION_SUCCESS);
        }
    }

    // Component Pools
    for (auto const &it : pSimulation->componentPools) {
        if (it.sType == sType) {
            *pRefCount = it.refCount;

            return to_foeResult(FOE_SIMULATION_SUCCESS);
        }
    }

    // Systems
    for (auto &it : pSimulation->systems) {
        if (it.sType == sType) {
            *pRefCount = it.refCount;

            return to_foeResult(FOE_SIMULATION_SUCCESS);
        }
    }

    return to_foeResult(FOE_SIMULATION_ERROR_TYPE_NOT_FOUND);
}

foeResultSet foeSimulationIncrementRefCount(foeSimulation *pSimulation,
                                            foeSimulationStructureType sType,
                                            size_t *pRefCount) {
    // Resource Loaders
    for (auto &it : pSimulation->resourceLoaders) {
        if (it.sType == sType) {
            if (pRefCount != nullptr)
                *pRefCount = ++it.refCount;
            else
                ++it.refCount;

            return to_foeResult(FOE_SIMULATION_SUCCESS);
        }
    }

    // Component Pools
    for (auto &it : pSimulation->componentPools) {
        if (it.sType == sType) {
            if (pRefCount != nullptr)
                *pRefCount = ++it.refCount;
            else
                ++it.refCount;

            return to_foeResult(FOE_SIMULATION_SUCCESS);
        }
    }

    // Systems
    for (auto &it : pSimulation->systems) {
        if (it.sType == sType) {
            if (pRefCount != nullptr)
                *pRefCount = ++it.refCount;
            else
                ++it.refCount;

            return to_foeResult(FOE_SIMULATION_SUCCESS);
        }
    }

    return to_foeResult(FOE_SIMULATION_ERROR_TYPE_NOT_FOUND);
}

foeResultSet foeSimulationDecrementRefCount(foeSimulation *pSimulation,
                                            foeSimulationStructureType sType,
                                            size_t *pRefCount) {
    // Resource Loaders
    for (auto &it : pSimulation->resourceLoaders) {
        if (it.sType == sType) {
            if (pRefCount != nullptr)
                *pRefCount = --it.refCount;
            else
                --it.refCount;

            return to_foeResult(FOE_SIMULATION_SUCCESS);
        }
    }

    // Component Pools
    for (auto &it : pSimulation->componentPools) {
        if (it.sType == sType) {
            if (pRefCount != nullptr)
                *pRefCount = --it.refCount;
            else
                --it.refCount;

            return to_foeResult(FOE_SIMULATION_SUCCESS);
        }
    }

    // Systems
    for (auto &it : pSimulation->systems) {
        if (it.sType == sType) {
            if (pRefCount != nullptr)
                *pRefCount = --it.refCount;
            else
                --it.refCount;

            return to_foeResult(FOE_SIMULATION_SUCCESS);
        }
    }

    return to_foeResult(FOE_SIMULATION_ERROR_TYPE_NOT_FOUND);
}

foeResultSet foeSimulationGetInitCount(foeSimulation const *pSimulation,
                                       foeSimulationStructureType sType,
                                       size_t *pInitCount) {
    // Resource Loaders
    for (auto const &it : pSimulation->resourceLoaders) {
        if (it.sType == sType) {
            *pInitCount = it.initCount;

            return to_foeResult(FOE_SIMULATION_SUCCESS);
        }
    }

    // Systems
    for (auto &it : pSimulation->systems) {
        if (it.sType == sType) {
            *pInitCount = it.initCount;

            return to_foeResult(FOE_SIMULATION_SUCCESS);
        }
    }

    return to_foeResult(FOE_SIMULATION_ERROR_TYPE_NOT_FOUND);
}

foeResultSet foeSimulationIncrementInitCount(foeSimulation *pSimulation,
                                             foeSimulationStructureType sType,
                                             size_t *pInitCount) {
    // Resource Loaders
    for (auto &it : pSimulation->resourceLoaders) {
        if (it.sType == sType) {
            if (pInitCount != nullptr)
                *pInitCount = ++it.initCount;
            else
                ++it.initCount;

            return to_foeResult(FOE_SIMULATION_SUCCESS);
        }
    }

    // Systems
    for (auto &it : pSimulation->systems) {
        if (it.sType == sType) {
            if (pInitCount != nullptr)
                *pInitCount = ++it.initCount;
            else
                ++it.initCount;

            return to_foeResult(FOE_SIMULATION_SUCCESS);
        }
    }

    return to_foeResult(FOE_SIMULATION_ERROR_TYPE_NOT_FOUND);
}

foeResultSet foeSimulationDecrementInitCount(foeSimulation *pSimulation,
                                             foeSimulationStructureType sType,
                                             size_t *pInitCount) {
    // Resource Loaders
    for (auto &it : pSimulation->resourceLoaders) {
        if (it.sType == sType) {
            if (pInitCount != nullptr)
                *pInitCount = --it.initCount;
            else
                --it.initCount;

            return to_foeResult(FOE_SIMULATION_SUCCESS);
        }
    }

    // Systems
    for (auto &it : pSimulation->systems) {
        if (it.sType == sType) {
            if (pInitCount != nullptr)
                *pInitCount = --it.initCount;
            else
                --it.initCount;

            return to_foeResult(FOE_SIMULATION_SUCCESS);
        }
    }

    return to_foeResult(FOE_SIMULATION_ERROR_TYPE_NOT_FOUND);
}

foeResultSet foeSimulationGetGfxInitCount(foeSimulation const *pSimulation,
                                          foeSimulationStructureType sType,
                                          size_t *pGfxInitCount) {
    // Resource Loaders
    for (auto const &it : pSimulation->resourceLoaders) {
        if (it.sType == sType) {
            *pGfxInitCount = it.gfxInitCount;

            return to_foeResult(FOE_SIMULATION_SUCCESS);
        }
    }

    // Systems
    for (auto &it : pSimulation->systems) {
        if (it.sType == sType) {
            *pGfxInitCount = it.gfxInitCount;

            return to_foeResult(FOE_SIMULATION_SUCCESS);
        }
    }

    return to_foeResult(FOE_SIMULATION_ERROR_TYPE_NOT_FOUND);
}

foeResultSet foeSimulationIncrementGfxInitCount(foeSimulation *pSimulation,
                                                foeSimulationStructureType sType,
                                                size_t *pGfxInitCount) {
    // Resource Loaders
    for (auto &it : pSimulation->resourceLoaders) {
        if (it.sType == sType) {
            if (pGfxInitCount != nullptr)
                *pGfxInitCount = ++it.gfxInitCount;
            else
                ++it.gfxInitCount;

            return to_foeResult(FOE_SIMULATION_SUCCESS);
        }
    }

    // Systems
    for (auto &it : pSimulation->systems) {
        if (it.sType == sType) {
            if (pGfxInitCount != nullptr)
                *pGfxInitCount = ++it.gfxInitCount;
            else
                ++it.gfxInitCount;

            return to_foeResult(FOE_SIMULATION_SUCCESS);
        }
    }

    return to_foeResult(FOE_SIMULATION_ERROR_TYPE_NOT_FOUND);
}

foeResultSet foeSimulationDecrementGfxInitCount(foeSimulation *pSimulation,
                                                foeSimulationStructureType sType,
                                                size_t *pGfxInitCount) {
    // Resource Loaders
    for (auto &it : pSimulation->resourceLoaders) {
        if (it.sType == sType) {
            if (pGfxInitCount != nullptr)
                *pGfxInitCount = --it.gfxInitCount;
            else
                --it.gfxInitCount;

            return to_foeResult(FOE_SIMULATION_SUCCESS);
        }
    }

    // Systems
    for (auto &it : pSimulation->systems) {
        if (it.sType == sType) {
            if (pGfxInitCount != nullptr)
                *pGfxInitCount = --it.gfxInitCount;
            else
                --it.gfxInitCount;

            return to_foeResult(FOE_SIMULATION_SUCCESS);
        }
    }

    return to_foeResult(FOE_SIMULATION_ERROR_TYPE_NOT_FOUND);
}

foeResultSet foeSimulationInsertResourceLoader(foeSimulation *pSimulation,
                                               foeSimulationLoaderData const *pCreateInfo) {
    // Make sure the type doesn't exist yet
    if (foeSimulationGetResourceLoader(pSimulation, pCreateInfo->sType) != nullptr ||
        foeSimulationGetSystem(pSimulation, pCreateInfo->sType) != nullptr ||
        foeSimulationGetComponentPool(pSimulation, pCreateInfo->sType) != nullptr) {
        return to_foeResult(FOE_SIMULATION_ERROR_TYPE_ALREADY_EXISTS);
    }

    pSimulation->resourceLoaders.emplace_back(*pCreateInfo);

    return to_foeResult(FOE_SIMULATION_SUCCESS);
}

foeResultSet foeSimulationReleaseResourceLoader(foeSimulation *pSimulation,
                                                foeSimulationStructureType sType,
                                                void **ppLoader) {
    auto const endIt = pSimulation->resourceLoaders.end();
    for (auto it = pSimulation->resourceLoaders.begin(); it != endIt; ++it) {
        if (it->sType == sType) {
            *ppLoader = it->pLoader;
            pSimulation->resourceLoaders.erase(it);

            return to_foeResult(FOE_SIMULATION_SUCCESS);
        }
    }

    return to_foeResult(FOE_SIMULATION_ERROR_TYPE_NOT_FOUND);
}

foeResultSet foeSimulationInsertComponentPool(foeSimulation *pSimulation,
                                              foeSimulationComponentPoolData const *pCreateInfo) {
    // Make sure the type doesn't exist yet
    if (foeSimulationGetResourceLoader(pSimulation, pCreateInfo->sType) != nullptr ||
        foeSimulationGetSystem(pSimulation, pCreateInfo->sType) != nullptr ||
        foeSimulationGetComponentPool(pSimulation, pCreateInfo->sType) != nullptr) {
        return to_foeResult(FOE_SIMULATION_ERROR_TYPE_ALREADY_EXISTS);
    }

    pSimulation->componentPools.emplace_back(*pCreateInfo);

    return to_foeResult(FOE_SIMULATION_SUCCESS);
}

foeResultSet foeSimulationReleaseComponentPool(foeSimulation *pSimulation,
                                               foeSimulationStructureType sType,
                                               void **ppComponentPool) {
    auto const endIt = pSimulation->componentPools.end();
    for (auto it = pSimulation->componentPools.begin(); it != endIt; ++it) {
        if (it->sType == sType) {
            *ppComponentPool = it->pComponentPool;
            pSimulation->componentPools.erase(it);

            return to_foeResult(FOE_SIMULATION_SUCCESS);
        }
    }

    return to_foeResult(FOE_SIMULATION_ERROR_TYPE_NOT_FOUND);
}

foeResultSet foeSimulationInsertSystem(foeSimulation *pSimulation,
                                       foeSimulationSystemData const *pCreateInfo) {
    // Make sure the type doesn't exist yet
    if (foeSimulationGetResourceLoader(pSimulation, pCreateInfo->sType) != nullptr ||
        foeSimulationGetSystem(pSimulation, pCreateInfo->sType) != nullptr ||
        foeSimulationGetComponentPool(pSimulation, pCreateInfo->sType) != nullptr) {
        return to_foeResult(FOE_SIMULATION_ERROR_TYPE_ALREADY_EXISTS);
    }

    pSimulation->systems.emplace_back(*pCreateInfo);

    return to_foeResult(FOE_SIMULATION_SUCCESS);
}

foeResultSet foeSimulationReleaseSystem(foeSimulation *pSimulation,
                                        foeSimulationStructureType sType,
                                        void **ppSystem) {
    auto const endIt = pSimulation->systems.end();
    for (auto it = pSimulation->systems.begin(); it != endIt; ++it) {
        if (it->sType == sType) {
            *ppSystem = it->pSystem;
            pSimulation->systems.erase(it);

            return to_foeResult(FOE_SIMULATION_SUCCESS);
        }
    }

    return to_foeResult(FOE_SIMULATION_ERROR_TYPE_NOT_FOUND);
}

foeResultSet foeSimulationGetResourceCreateInfo(foeSimulation const *pSimulation,
                                                foeResourceID resourceID,
                                                foeResourceCreateInfo *pResourceCI) {
    auto ci = getResourceCreateInfo((void *)pSimulation, resourceID);

    if (ci == FOE_NULL_HANDLE) {
        // @todo Replace with proper result code
        std::abort();
    }

    *pResourceCI = ci;
    return to_foeResult(FOE_SIMULATION_SUCCESS);
}