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

#include "error_code.hpp"
#include "log.hpp"

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

auto foeRegisterFunctionality(foeSimulationFunctionalty const &functionality) -> std::error_code {
    std::scoped_lock lock{mSync};

    if (functionality.id < 1000000000 || functionality.id % 1000 != 0) {
        FOE_LOG(SimulationState, Warning,
                "foeRegisterFunctionality - Attempted to register functionality with invalid ID");
        return FOE_SIMULATION_ERROR_ID_INVALID;
    }

    for (auto const &it : mRegistered) {
        if (it.id == functionality.id) {
            FOE_LOG(SimulationState, Warning,
                    "foeRegisterFunctionality - Attempted to register functionality with an ID "
                    "already in use");
            return FOE_SIMULATION_ERROR_ID_ALREADY_IN_USE;
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

    std::error_code errC;
    foeSimulation **ppSimState = mStates.data();
    foeSimulation **ppEndSimState = mStates.data() + mStates.size();

    for (; ppSimState != ppEndSimState; ++ppSimState) {
        // Reset the set of passed calls
        passedStates = {};

        acquireExclusiveLock(*ppSimState, "functionality registration");

        if (functionality.pCreateFn) {
            errC = functionality.pCreateFn(*ppSimState);
            if (errC) {
                FOE_LOG(SimulationState, Error,
                        "foeRegisterFunctionality - Failed creating functionality on "
                        "SimulationState: {} due "
                        "to error: {}",
                        static_cast<void *>(*ppSimState), errC.message());
                break;
            }
            passedStates.created = true;
        }
        if (foeSimulationIsInitialized(*ppSimState) && functionality.pInitializeFn) {
            errC = functionality.pInitializeFn(*ppSimState, &(*ppSimState)->initInfo);
            if (errC) {
                FOE_LOG(SimulationState, Error,
                        "foeRegisterFunctionality - Failed initializing functionality on "
                        "SimulationState: {} due to error: {}",
                        static_cast<void *>(*ppSimState), errC.message());
                break;
            }
            passedStates.initialized = true;
        }
        if (foeSimulationIsGraphicsInitialzied(*ppSimState) &&
            functionality.pInitializeGraphicsFn) {
            errC = functionality.pInitializeGraphicsFn(*ppSimState, (*ppSimState)->gfxSession);
            if (errC) {
                FOE_LOG(SimulationState, Error,
                        "foeRegisterFunctionality - Failed initializing graphics functionality on "
                        "SimulationState: {} due to error: {}",
                        static_cast<void *>(*ppSimState), errC.message());
                break;
            }
            passedStates.gfxInitialized = true;
        }

        (*ppSimState)->simSync.unlock();
        if (errC)
            break;
    }

    // If we failed to initialize in one of the simulations above, then we need to go through and
    // deinitialize/destroy this functionality in all the ones it was already added to successfully
    if (errC && ppSimState != mStates.data()) {
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

        return errC;
    }

    return FOE_SIMULATION_SUCCESS;
}

auto foeDeregisterFunctionality(foeSimulationUUID functionalityUUID) -> std::error_code {
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
            return FOE_SIMULATION_SUCCESS;
        }
    }

    FOE_LOG(
        SimulationState, Warning,
        "registerFunctionality - Attempted to deregister functionality that was never registered");
    return FOE_SIMULATION_ERROR_NOT_REGISTERED;
}

bool foeSimulationIsInitialized(foeSimulation const *pSimulation) {
    return !!pSimulation->initInfo.externalFileSearchFn;
}

bool foeSimulationIsGraphicsInitialzied(foeSimulation const *pSimulation) {
    return pSimulation->gfxSession != FOE_NULL_HANDLE;
}

auto foeCreateSimulation(bool addNameMaps, foeSimulation **ppSimulationState) -> std::error_code {
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

    std::error_code errC = foeCreateResourcePool(&resourceCallbacks, &newSimState->resourcePool);
    if (errC)
        return errC;

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
            errC = it->pCreateFn(newSimState.get());
            if (errC) {
                FOE_LOG(SimulationState, Error,
                        "[{}] foeSimulation - Error creating due to error: {}",
                        static_cast<void *>(newSimState.get()), errC.message());
                break;
            }
        }
    }

    // If there is an error code at this point, then we need to walk-back what was created so far
    if (errC && it != mRegistered.begin()) {
        --it;
        for (; it >= mRegistered.begin(); --it) {
            it->pDestroyFn(newSimState.get());
        }
    }

    if (!errC) {
        FOE_LOG(SimulationState, Verbose, "[{}] foeSimulation - Created",
                static_cast<void *>(newSimState.get()));
        mStates.emplace_back(newSimState.get());
        *ppSimulationState = newSimState.release();
    } else {
        foeDestroySimulation(newSimState.get());
    }

    return errC;
}

auto foeDestroySimulation(foeSimulation *pSimulation) -> std::error_code {
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
        return FOE_SIMULATION_ERROR_NOT_REGISTERED;
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

    return FOE_SIMULATION_SUCCESS;
}

auto foeInitializeSimulation(foeSimulation *pSimulation, foeSimulationInitInfo const *pInitInfo)
    -> std::error_code {
    std::scoped_lock lock{mSync};

    if (foeSimulationIsInitialized(pSimulation)) {
        FOE_LOG(SimulationState, Error, "[{}] foeSimulation - Attempted to re-initialize",
                static_cast<void *>(pSimulation))
        return FOE_SIMULATION_ERROR_SIMULATION_ALREADY_INITIALIZED;
    }

    FOE_LOG(SimulationState, Verbose, "[{}] foeSimulation - Initializing",
            static_cast<void *>(pSimulation));

    acquireExclusiveLock(pSimulation, "initialization");

    // Go through each set of functionality and call the initialization function, if one is attached
    std::error_code errC{FOE_SIMULATION_SUCCESS};
    auto it = mRegistered.begin();
    auto const endIt = mRegistered.end();

    for (; it != endIt; ++it) {
        if (it->pInitializeFn) {
            errC = it->pInitializeFn(pSimulation, pInitInfo);
            if (errC) {
                FOE_LOG(SimulationState, Error,
                        "[{}] foeSimulation - Failed to initialize due to error: {}",
                        static_cast<void *>(pSimulation), errC.message());
                break;
            }
        }
    }

    // If there is an error code at this point, then we need to walk-back what was created so far
    if (errC && it != mRegistered.begin()) {
        --it;
        for (; it >= mRegistered.begin(); --it) {
            if (it->pDeinitializeFn) {
                it->pDeinitializeFn(pSimulation);
            }
        }
    }

    if (!errC) {
        // On a successful initialization, set it into the state itself
        pSimulation->initInfo = *pInitInfo;
        FOE_LOG(SimulationState, Verbose, "[{}] foeSimulation - Initialized",
                static_cast<void *>(pSimulation));
    }

    pSimulation->simSync.unlock();

    return errC;
}

auto foeDeinitializeSimulation(foeSimulation *pSimulation) -> std::error_code {
    std::scoped_lock lock{mSync};

    if (!foeSimulationIsInitialized(pSimulation))
        return FOE_SIMULATION_ERROR_SIMULATION_NOT_INITIALIZED;

    acquireExclusiveLock(pSimulation, "deinitialization");
    deinitSimulation(pSimulation);
    return FOE_SIMULATION_SUCCESS;
}

auto foeInitializeSimulationGraphics(foeSimulation *pSimulation, foeGfxSession gfxSession)
    -> std::error_code {
    std::scoped_lock lock{mSync};

    if (foeSimulationIsGraphicsInitialzied(pSimulation)) {
        FOE_LOG(SimulationState, Error, "[{}] foeSimulation - Attempted to re-initialize graphics",
                static_cast<void *>(pSimulation))
        return FOE_SIMULATION_ERROR_SIMULATION_GRAPHICS_ALREADY_INITIALIZED;
    }

    FOE_LOG(SimulationState, Verbose, "[{}] foeSimulation - Initializing graphics",
            static_cast<void *>(pSimulation));

    acquireExclusiveLock(pSimulation, "initializing graphics");

    // Go through each set of functionality and call the initialization function, if one is attached
    std::error_code errC{FOE_SIMULATION_SUCCESS};
    auto it = mRegistered.begin();
    auto const endIt = mRegistered.end();

    for (; it != endIt; ++it) {
        if (it->pInitializeGraphicsFn) {
            errC = it->pInitializeGraphicsFn(pSimulation, gfxSession);
            if (errC) {
                FOE_LOG(SimulationState, Error,
                        "[{}] foeSimulation - Failed to initialize graphics due to error: {}",
                        static_cast<void *>(pSimulation), errC.message());
                break;
            }
        }
    }

    // If there is an error code at this point, then we need to walk-back what was created so far
    if (errC && it != mRegistered.begin()) {
        --it;
        for (; it >= mRegistered.begin(); --it) {
            if (it->pDeinitializeGraphicsFn) {
                it->pDeinitializeGraphicsFn(pSimulation);
            }
        }
    }

    if (!errC) {
        // With success, set the simulation's graphics session handle
        pSimulation->gfxSession = gfxSession;
        FOE_LOG(SimulationState, Verbose, "[{}] foeSimulation - Initialized graphics",
                static_cast<void *>(pSimulation));
    }

    pSimulation->simSync.unlock();

    return errC;
}

auto foeDeinitializeSimulationGraphics(foeSimulation *pSimulation) -> std::error_code {
    std::scoped_lock lock{mSync};

    if (!foeSimulationIsGraphicsInitialzied(pSimulation)) {
        FOE_LOG(SimulationState, Warning,
                "[{}] foeSimulation - Attempted to deinitialize uninitialized graphics");
        return FOE_SIMULATION_ERROR_SIMULATION_GRAPHICS_NOT_INITIALIZED;
    }

    acquireExclusiveLock(pSimulation, "deinitializing graphics");
    deinitializeSimulationGraphics(pSimulation);
    return FOE_SIMULATION_SUCCESS;
}

auto foeSimulationGetRefCount(foeSimulation const *pSimulation,
                              foeSimulationStructureType sType,
                              size_t *pRefCount) -> std::error_code {
    // Resource Loaders
    for (auto const &it : pSimulation->resourceLoaders) {
        if (it.sType == sType) {
            *pRefCount = it.refCount;

            return FOE_SIMULATION_SUCCESS;
        }
    }

    // Component Pools
    for (auto const &it : pSimulation->componentPools) {
        if (it.sType == sType) {
            *pRefCount = it.refCount;

            return FOE_SIMULATION_SUCCESS;
        }
    }

    // Systems
    for (auto &it : pSimulation->systems) {
        if (it.sType == sType) {
            *pRefCount = it.refCount;

            return FOE_SIMULATION_SUCCESS;
        }
    }

    return FOE_SIMULATION_ERROR_TYPE_NOT_FOUND;
}

auto foeSimulationIncrementRefCount(foeSimulation *pSimulation,
                                    foeSimulationStructureType sType,
                                    size_t *pRefCount) -> std::error_code {
    // Resource Loaders
    for (auto &it : pSimulation->resourceLoaders) {
        if (it.sType == sType) {
            if (pRefCount != nullptr)
                *pRefCount = ++it.refCount;
            else
                ++it.refCount;

            return FOE_SIMULATION_SUCCESS;
        }
    }

    // Component Pools
    for (auto &it : pSimulation->componentPools) {
        if (it.sType == sType) {
            if (pRefCount != nullptr)
                *pRefCount = ++it.refCount;
            else
                ++it.refCount;

            return FOE_SIMULATION_SUCCESS;
        }
    }

    // Systems
    for (auto &it : pSimulation->systems) {
        if (it.sType == sType) {
            if (pRefCount != nullptr)
                *pRefCount = ++it.refCount;
            else
                ++it.refCount;

            return FOE_SIMULATION_SUCCESS;
        }
    }

    return FOE_SIMULATION_ERROR_TYPE_NOT_FOUND;
}

auto foeSimulationDecrementRefCount(foeSimulation *pSimulation,
                                    foeSimulationStructureType sType,
                                    size_t *pRefCount) -> std::error_code {
    // Resource Loaders
    for (auto &it : pSimulation->resourceLoaders) {
        if (it.sType == sType) {
            if (pRefCount != nullptr)
                *pRefCount = --it.refCount;
            else
                --it.refCount;

            return FOE_SIMULATION_SUCCESS;
        }
    }

    // Component Pools
    for (auto &it : pSimulation->componentPools) {
        if (it.sType == sType) {
            if (pRefCount != nullptr)
                *pRefCount = --it.refCount;
            else
                --it.refCount;

            return FOE_SIMULATION_SUCCESS;
        }
    }

    // Systems
    for (auto &it : pSimulation->systems) {
        if (it.sType == sType) {
            if (pRefCount != nullptr)
                *pRefCount = --it.refCount;
            else
                --it.refCount;

            return FOE_SIMULATION_SUCCESS;
        }
    }

    return FOE_SIMULATION_ERROR_TYPE_NOT_FOUND;
}

auto foeSimulationGetInitCount(foeSimulation const *pSimulation,
                               foeSimulationStructureType sType,
                               size_t *pInitCount) -> std::error_code {
    // Resource Loaders
    for (auto const &it : pSimulation->resourceLoaders) {
        if (it.sType == sType) {
            *pInitCount = it.initCount;

            return FOE_SIMULATION_SUCCESS;
        }
    }

    // Systems
    for (auto &it : pSimulation->systems) {
        if (it.sType == sType) {
            *pInitCount = it.initCount;

            return FOE_SIMULATION_SUCCESS;
        }
    }

    return FOE_SIMULATION_ERROR_TYPE_NOT_FOUND;
}

auto foeSimulationIncrementInitCount(foeSimulation *pSimulation,
                                     foeSimulationStructureType sType,
                                     size_t *pInitCount) -> std::error_code {
    // Resource Loaders
    for (auto &it : pSimulation->resourceLoaders) {
        if (it.sType == sType) {
            if (pInitCount != nullptr)
                *pInitCount = ++it.initCount;
            else
                ++it.initCount;

            return FOE_SIMULATION_SUCCESS;
        }
    }

    // Systems
    for (auto &it : pSimulation->systems) {
        if (it.sType == sType) {
            if (pInitCount != nullptr)
                *pInitCount = ++it.initCount;
            else
                ++it.initCount;

            return FOE_SIMULATION_SUCCESS;
        }
    }

    return FOE_SIMULATION_ERROR_TYPE_NOT_FOUND;
}

auto foeSimulationDecrementInitCount(foeSimulation *pSimulation,
                                     foeSimulationStructureType sType,
                                     size_t *pInitCount) -> std::error_code {
    // Resource Loaders
    for (auto &it : pSimulation->resourceLoaders) {
        if (it.sType == sType) {
            if (pInitCount != nullptr)
                *pInitCount = --it.initCount;
            else
                --it.initCount;

            return FOE_SIMULATION_SUCCESS;
        }
    }

    // Systems
    for (auto &it : pSimulation->systems) {
        if (it.sType == sType) {
            if (pInitCount != nullptr)
                *pInitCount = --it.initCount;
            else
                --it.initCount;

            return FOE_SIMULATION_SUCCESS;
        }
    }

    return FOE_SIMULATION_ERROR_TYPE_NOT_FOUND;
}

auto foeSimulationGetGfxInitCount(foeSimulation const *pSimulation,
                                  foeSimulationStructureType sType,
                                  size_t *pGfxInitCount) -> std::error_code {
    // Resource Loaders
    for (auto const &it : pSimulation->resourceLoaders) {
        if (it.sType == sType) {
            *pGfxInitCount = it.gfxInitCount;

            return FOE_SIMULATION_SUCCESS;
        }
    }

    // Systems
    for (auto &it : pSimulation->systems) {
        if (it.sType == sType) {
            *pGfxInitCount = it.gfxInitCount;

            return FOE_SIMULATION_SUCCESS;
        }
    }

    return FOE_SIMULATION_ERROR_TYPE_NOT_FOUND;
}

auto foeSimulationIncrementGfxInitCount(foeSimulation *pSimulation,
                                        foeSimulationStructureType sType,
                                        size_t *pGfxInitCount) -> std::error_code {
    // Resource Loaders
    for (auto &it : pSimulation->resourceLoaders) {
        if (it.sType == sType) {
            if (pGfxInitCount != nullptr)
                *pGfxInitCount = ++it.gfxInitCount;
            else
                ++it.gfxInitCount;

            return FOE_SIMULATION_SUCCESS;
        }
    }

    // Systems
    for (auto &it : pSimulation->systems) {
        if (it.sType == sType) {
            if (pGfxInitCount != nullptr)
                *pGfxInitCount = ++it.gfxInitCount;
            else
                ++it.gfxInitCount;

            return FOE_SIMULATION_SUCCESS;
        }
    }

    return FOE_SIMULATION_ERROR_TYPE_NOT_FOUND;
}

auto foeSimulationDecrementGfxInitCount(foeSimulation *pSimulation,
                                        foeSimulationStructureType sType,
                                        size_t *pGfxInitCount) -> std::error_code {
    // Resource Loaders
    for (auto &it : pSimulation->resourceLoaders) {
        if (it.sType == sType) {
            if (pGfxInitCount != nullptr)
                *pGfxInitCount = --it.gfxInitCount;
            else
                --it.gfxInitCount;

            return FOE_SIMULATION_SUCCESS;
        }
    }

    // Systems
    for (auto &it : pSimulation->systems) {
        if (it.sType == sType) {
            if (pGfxInitCount != nullptr)
                *pGfxInitCount = --it.gfxInitCount;
            else
                --it.gfxInitCount;

            return FOE_SIMULATION_SUCCESS;
        }
    }

    return FOE_SIMULATION_ERROR_TYPE_NOT_FOUND;
}

auto foeSimulationInsertResourceLoader(foeSimulation *pSimulation,
                                       foeSimulationLoaderData const *pCreateInfo)
    -> std::error_code {
    // Make sure the type doesn't exist yet
    if (foeSimulationGetResourceLoader(pSimulation, pCreateInfo->sType) != nullptr ||
        foeSimulationGetSystem(pSimulation, pCreateInfo->sType) != nullptr ||
        foeSimulationGetComponentPool(pSimulation, pCreateInfo->sType) != nullptr) {
        return FOE_SIMULATION_ERROR_TYPE_ALREADY_EXISTS;
    }

    pSimulation->resourceLoaders.emplace_back(*pCreateInfo);

    return FOE_SIMULATION_SUCCESS;
}

auto foeSimulationReleaseResourceLoader(foeSimulation *pSimulation,
                                        foeSimulationStructureType sType,
                                        void **ppLoader) -> std::error_code {
    auto const endIt = pSimulation->resourceLoaders.end();
    for (auto it = pSimulation->resourceLoaders.begin(); it != endIt; ++it) {
        if (it->sType == sType) {
            *ppLoader = it->pLoader;
            pSimulation->resourceLoaders.erase(it);

            return FOE_SIMULATION_SUCCESS;
        }
    }

    return FOE_SIMULATION_ERROR_TYPE_NOT_FOUND;
}

auto foeSimulationInsertComponentPool(foeSimulation *pSimulation,
                                      foeSimulationComponentPoolData const *pCreateInfo)
    -> std::error_code {
    // Make sure the type doesn't exist yet
    if (foeSimulationGetResourceLoader(pSimulation, pCreateInfo->sType) != nullptr ||
        foeSimulationGetSystem(pSimulation, pCreateInfo->sType) != nullptr ||
        foeSimulationGetComponentPool(pSimulation, pCreateInfo->sType) != nullptr) {
        return FOE_SIMULATION_ERROR_TYPE_ALREADY_EXISTS;
    }

    pSimulation->componentPools.emplace_back(*pCreateInfo);

    return FOE_SIMULATION_SUCCESS;
}

auto foeSimulationReleaseComponentPool(foeSimulation *pSimulation,
                                       foeSimulationStructureType sType,
                                       void **ppComponentPool) -> std::error_code {
    auto const endIt = pSimulation->componentPools.end();
    for (auto it = pSimulation->componentPools.begin(); it != endIt; ++it) {
        if (it->sType == sType) {
            *ppComponentPool = it->pComponentPool;
            pSimulation->componentPools.erase(it);

            return FOE_SIMULATION_SUCCESS;
        }
    }

    return FOE_SIMULATION_ERROR_TYPE_NOT_FOUND;
}

auto foeSimulationInsertSystem(foeSimulation *pSimulation,
                               foeSimulationSystemData const *pCreateInfo)
    -> std::error_code { // Make sure the type doesn't exist yet
    if (foeSimulationGetResourceLoader(pSimulation, pCreateInfo->sType) != nullptr ||
        foeSimulationGetSystem(pSimulation, pCreateInfo->sType) != nullptr ||
        foeSimulationGetComponentPool(pSimulation, pCreateInfo->sType) != nullptr) {
        return FOE_SIMULATION_ERROR_TYPE_ALREADY_EXISTS;
    }

    pSimulation->systems.emplace_back(*pCreateInfo);

    return FOE_SIMULATION_SUCCESS;
}

FOE_SIM_EXPORT auto foeSimulationReleaseSystem(foeSimulation *pSimulation,
                                               foeSimulationStructureType sType,
                                               void **ppSystem) -> std::error_code {
    auto const endIt = pSimulation->systems.end();
    for (auto it = pSimulation->systems.begin(); it != endIt; ++it) {
        if (it->sType == sType) {
            *ppSystem = it->pSystem;
            pSimulation->systems.erase(it);

            return FOE_SIMULATION_SUCCESS;
        }
    }

    return FOE_SIMULATION_ERROR_TYPE_NOT_FOUND;
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

    pPostLoadFn(resource, foeToErrorCode(FOE_SIMULATION_ERROR_NO_LOADER_FOUND), nullptr, nullptr,
                nullptr, nullptr, nullptr);
}