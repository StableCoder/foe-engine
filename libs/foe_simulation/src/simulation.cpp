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

#include <foe/simulation/registration.hpp>
#include <foe/simulation/simulation.hpp>

#include <foe/chrono/easy_clock.hpp>
#include <foe/ecs/editor_name_map.hpp>
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
    FOE_LOG(SimulationState, Verbose, "[{}] foeSimulation - Acquiring exclusive lock for {}",
            static_cast<void *>(pSimulation), pReason)
    foeEasyHighResClock waitingTime;

    pSimulation->simSync.lock();

    waitingTime.update();
    FOE_LOG(SimulationState, Verbose,
            "[{}] foeSimulation - Acquired exclusive lock for {} after {}ms",
            static_cast<void *>(pSimulation), pReason,
            waitingTime.elapsed<std::chrono::milliseconds>().count())
}

// Assumes that the mutex has already been acquired
void deinitSimulation(foeSimulation *pSimulation) {
    FOE_LOG(SimulationState, Verbose, "[{}] foeSimulation - Deinitializing",
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

    FOE_LOG(SimulationState, Verbose, "[{}] foeSimulation - Deinitialized",
            static_cast<void *>(pSimulation));
}

// Assumes that the mutex has already been acquired
void deinitializeSimulationGraphics(foeSimulation *pSimulation) {
    FOE_LOG(SimulationState, Verbose, "[{}] foeSimulation - Deinitializing Graphics",
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

    FOE_LOG(SimulationState, Verbose, "[{}] foeSimulation - Deinitialized Graphics",
            static_cast<void *>(pSimulation));
}

} // namespace

foeResult foeRegisterFunctionality(foeSimulationFunctionalty const &functionality) {
    std::scoped_lock lock{mSync};

    if (functionality.id < 1000000000 || functionality.id % 1000 != 0) {
        FOE_LOG(SimulationState, Warning,
                "foeRegisterFunctionality - Attempted to register functionality with invalid ID");
        return to_foeResult(FOE_SIMULATION_ERROR_ID_INVALID);
    }

    for (auto const &it : mRegistered) {
        if (it.id == functionality.id) {
            FOE_LOG(SimulationState, Warning,
                    "foeRegisterFunctionality - Attempted to register functionality with an ID "
                    "already in use");
            return to_foeResult(FOE_SIMULATION_ERROR_ID_ALREADY_IN_USE);
        }
    }

    // Not already registered, add it.
    mRegistered.emplace_back(functionality);

    // Go through any already existing SimulationState's and add this new functionality to them.
    struct {
        bool created;
        bool initialized;
        bool gfxInitialized;
    } passedStates;

    foeResult result = to_foeResult(FOE_SIMULATION_SUCCESS);
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
                FOE_LOG(SimulationState, Error,
                        "foeRegisterFunctionality - Failed creating functionality on "
                        "SimulationState: {} due "
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
                FOE_LOG(SimulationState, Error,
                        "foeRegisterFunctionality - Failed initializing functionality on "
                        "SimulationState: {} due to error: {}",
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
                FOE_LOG(SimulationState, Error,
                        "foeRegisterFunctionality - Failed initializing graphics functionality on "
                        "SimulationState: {} due to error: {}",
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

foeResult foeDeregisterFunctionality(foeSimulationUUID functionalityUUID) {
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
        SimulationState, Warning,
        "registerFunctionality - Attempted to deregister functionality that was never registered");
    return to_foeResult(FOE_SIMULATION_ERROR_NOT_REGISTERED);
}

bool foeSimulationIsInitialized(foeSimulation const *pSimulation) {
    return !!pSimulation->initInfo.externalFileSearchFn;
}

bool foeSimulationIsGraphicsInitialzied(foeSimulation const *pSimulation) {
    return pSimulation->gfxSession != FOE_NULL_HANDLE;
}

foeResult foeCreateSimulation(bool addNameMaps, foeSimulation **ppSimulationState) {
    std::scoped_lock lock{mSync};

    std::unique_ptr<foeSimulation> newSimState{new foeSimulation};
    newSimState->gfxSession = FOE_NULL_HANDLE;
    newSimState->resourcePool = FOE_NULL_HANDLE;

    // Resource Pool
    foeResourceFns resourceCallbacks{
        .pImportContext = &newSimState->groupData,
        .pImportFn = TEMP_foeSimulationGetResourceCreateInfo,
        .pLoadContext = newSimState.get(),
        .pLoadFn = TEMP_foeSimulationLoadResource,
    };

    foeResult result = foeCreateResourcePool(&resourceCallbacks, &newSimState->resourcePool);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(SimulationState, Error, "foeSimulation - Failed to create ResourcePool due to: {}",
                buffer);

        return result;
    }

    // Editor Name Maps, if requested
    if (addNameMaps) {
        newSimState->pResourceNameMap = new foeEditorNameMap;
        newSimState->pEntityNameMap = new foeEditorNameMap;
    } else {
        newSimState->pResourceNameMap = nullptr;
        newSimState->pEntityNameMap = nullptr;
    }

    FOE_LOG(SimulationState, Verbose, "[{}] foeSimulation - Creating",
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
                FOE_LOG(SimulationState, Error,
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
        FOE_LOG(SimulationState, Verbose, "[{}] foeSimulation - Created",
                static_cast<void *>(newSimState.get()));
        mStates.emplace_back(newSimState.get());
        *ppSimulationState = newSimState.release();
    } else {
        foeDestroySimulation(newSimState.get());
    }

    return result;
}

foeResult foeDestroySimulation(foeSimulation *pSimulation) {
    std::scoped_lock lock{mSync};

    FOE_LOG(SimulationState, Verbose, "[{}] foeSimulation - Destroying",
            static_cast<void *>(pSimulation));

    auto searchIt = std::find(mStates.begin(), mStates.end(), pSimulation);
    if (searchIt == mStates.end()) {
        // We were given a simulation state that wasn't created from here?
        FOE_LOG(SimulationState, Warning,
                "[{}] foeSimulation - Given a SimulationState that wasn't created via "
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
    if (pSimulation->pEntityNameMap)
        delete pSimulation->pEntityNameMap;
    if (pSimulation->pResourceNameMap)
        delete pSimulation->pResourceNameMap;

    // Destroy ResourcePool
    foeDestroyResourcePool(pSimulation->resourcePool);

    // Delete it
    delete pSimulation;

    FOE_LOG(SimulationState, Verbose, "[{}] foeSimulation - Destroyed",
            static_cast<void *>(pSimulation));

    return to_foeResult(FOE_SIMULATION_SUCCESS);
}

foeResult foeInitializeSimulation(foeSimulation *pSimulation,
                                  foeSimulationInitInfo const *pInitInfo) {
    std::scoped_lock lock{mSync};

    if (foeSimulationIsInitialized(pSimulation)) {
        FOE_LOG(SimulationState, Error, "[{}] foeSimulation - Attempted to re-initialize",
                static_cast<void *>(pSimulation))
        return to_foeResult(FOE_SIMULATION_ERROR_SIMULATION_ALREADY_INITIALIZED);
    }

    FOE_LOG(SimulationState, Verbose, "[{}] foeSimulation - Initializing",
            static_cast<void *>(pSimulation));

    acquireExclusiveLock(pSimulation, "initialization");

    // Go through each set of functionality and call the initialization function, if one is attached
    foeResult result = to_foeResult(FOE_SIMULATION_SUCCESS);
    auto it = mRegistered.begin();
    auto const endIt = mRegistered.end();

    for (; it != endIt; ++it) {
        if (it->pInitializeFn) {
            result = it->pInitializeFn(pSimulation, pInitInfo);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(SimulationState, Error,
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
        FOE_LOG(SimulationState, Verbose, "[{}] foeSimulation - Initialized",
                static_cast<void *>(pSimulation));
    }

    pSimulation->simSync.unlock();

    return result;
}

foeResult foeDeinitializeSimulation(foeSimulation *pSimulation) {
    std::scoped_lock lock{mSync};

    if (!foeSimulationIsInitialized(pSimulation))
        return to_foeResult(FOE_SIMULATION_ERROR_SIMULATION_NOT_INITIALIZED);

    acquireExclusiveLock(pSimulation, "deinitialization");
    deinitSimulation(pSimulation);

    return to_foeResult(FOE_SIMULATION_SUCCESS);
}

foeResult foeInitializeSimulationGraphics(foeSimulation *pSimulation, foeGfxSession gfxSession) {
    std::scoped_lock lock{mSync};

    if (foeSimulationIsGraphicsInitialzied(pSimulation)) {
        FOE_LOG(SimulationState, Error, "[{}] foeSimulation - Attempted to re-initialize graphics",
                static_cast<void *>(pSimulation))
        return to_foeResult(FOE_SIMULATION_ERROR_SIMULATION_GRAPHICS_ALREADY_INITIALIZED);
    }

    FOE_LOG(SimulationState, Verbose, "[{}] foeSimulation - Initializing graphics",
            static_cast<void *>(pSimulation));

    acquireExclusiveLock(pSimulation, "initializing graphics");

    // Go through each set of functionality and call the initialization function, if one is attached
    foeResult result = to_foeResult(FOE_SIMULATION_SUCCESS);
    auto it = mRegistered.begin();
    auto const endIt = mRegistered.end();

    for (; it != endIt; ++it) {
        if (it->pInitializeGraphicsFn) {
            result = it->pInitializeGraphicsFn(pSimulation, gfxSession);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(SimulationState, Error,
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
        FOE_LOG(SimulationState, Verbose, "[{}] foeSimulation - Initialized graphics",
                static_cast<void *>(pSimulation));
    }

    pSimulation->simSync.unlock();

    return result;
}

foeResult foeDeinitializeSimulationGraphics(foeSimulation *pSimulation) {
    std::scoped_lock lock{mSync};

    if (!foeSimulationIsGraphicsInitialzied(pSimulation)) {
        FOE_LOG(SimulationState, Warning,
                "[{}] foeSimulation - Attempted to deinitialize uninitialized graphics");
        return to_foeResult(FOE_SIMULATION_ERROR_SIMULATION_GRAPHICS_NOT_INITIALIZED);
    }

    acquireExclusiveLock(pSimulation, "deinitializing graphics");
    deinitializeSimulationGraphics(pSimulation);
    return to_foeResult(FOE_SIMULATION_SUCCESS);
}

foeResult foeSimulationGetRefCount(foeSimulation const *pSimulation,
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

foeResult foeSimulationIncrementRefCount(foeSimulation *pSimulation,
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

foeResult foeSimulationDecrementRefCount(foeSimulation *pSimulation,
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

foeResult foeSimulationGetInitCount(foeSimulation const *pSimulation,
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

foeResult foeSimulationIncrementInitCount(foeSimulation *pSimulation,
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

foeResult foeSimulationDecrementInitCount(foeSimulation *pSimulation,
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

foeResult foeSimulationGetGfxInitCount(foeSimulation const *pSimulation,
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

foeResult foeSimulationIncrementGfxInitCount(foeSimulation *pSimulation,
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

foeResult foeSimulationDecrementGfxInitCount(foeSimulation *pSimulation,
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

foeResult foeSimulationInsertResourceLoader(foeSimulation *pSimulation,
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

foeResult foeSimulationReleaseResourceLoader(foeSimulation *pSimulation,
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

foeResult foeSimulationInsertComponentPool(foeSimulation *pSimulation,
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

foeResult foeSimulationReleaseComponentPool(foeSimulation *pSimulation,
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

foeResult foeSimulationInsertSystem(foeSimulation *pSimulation,
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

foeResult foeSimulationReleaseSystem(foeSimulation *pSimulation,
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

foeResourceCreateInfo TEMP_foeSimulationGetResourceCreateInfo(void *pContext,
                                                              foeResourceID resourceID) {
    auto *pGroupData = reinterpret_cast<foeGroupData *>(pContext);

    return pGroupData->getResourceDefinition(resourceID);
}

void TEMP_foeSimulationLoadResource(void *pContext,
                                    foeResource resource,
                                    PFN_foeResourcePostLoad *pPostLoadFn) {
    auto *pSimulation = reinterpret_cast<foeSimulation *>(pContext);

    auto createInfo = foeResourceGetCreateInfo(resource);

    for (auto const &it : pSimulation->resourceLoaders) {
        if (it.pCanProcessCreateInfoFn(createInfo)) {
            it.pLoadFn(it.pLoader, resource, createInfo, pPostLoadFn);
            return;
        }
    }

    pPostLoadFn(resource, to_foeResult(FOE_SIMULATION_ERROR_NO_LOADER_FOUND), nullptr, nullptr,
                nullptr, nullptr, nullptr);
}