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

#include "error_code.hpp"
#include "log.hpp"

#include <mutex>
#include <vector>

namespace {
std::mutex mSync;

std::vector<foeSimulationFunctionalty> mRegistered;
std::vector<foeSimulationState *> mStates;

void acquireExclusiveLock(foeSimulationState *pSimulationState, char const *pReason) {
    FOE_LOG(SimulationState, Verbose, "Acquiring exclusive lock for SimulationState {} for {}",
            static_cast<void *>(pSimulationState), pReason)
    foeEasyHighResClock waitingTime;

    pSimulationState->simSync.lock();

    waitingTime.update();
    FOE_LOG(SimulationState, Verbose,
            "Acquired exclusive lock for SimulationState {} for {} after {}ms",
            static_cast<void *>(pSimulationState), pReason,
            waitingTime.elapsed<std::chrono::milliseconds>().count())
}

// Assumes that the mutex has already been acquired
void deinitSimulation(foeSimulationState *pSimulationState) {
    FOE_LOG(SimulationState, Verbose, "Deinitializing SimulationState: {}",
            static_cast<void *>(pSimulationState));

    // Deinit functionality
    for (auto const &functionality : mRegistered) {
        if (functionality.onDeinitialization) {
            functionality.onDeinitialization(pSimulationState);
        }
    }

    pSimulationState->initInfo = {};

    pSimulationState->simSync.unlock();

    FOE_LOG(SimulationState, Verbose, "Deinitialized SimulationState: {}",
            static_cast<void *>(pSimulationState));
}

// Assumes that the mutex has already been acquired
void deinitializeSimulationGraphics(foeSimulationState *pSimulationState) {
    FOE_LOG(SimulationState, Verbose, "Deinitializing SimulationState Graphics: {}",
            static_cast<void *>(pSimulationState));

    // Iterate through all and deinitialize
    for (auto const &functionality : mRegistered) {
        if (functionality.onGfxDeinitialization) {
            functionality.onGfxDeinitialization(pSimulationState);
        }
    }

    pSimulationState->gfxSession = FOE_NULL_HANDLE;

    pSimulationState->simSync.unlock();

    FOE_LOG(SimulationState, Verbose, "Deinitialized SimulationState Graphics: {}",
            static_cast<void *>(pSimulationState));
}

} // namespace

bool foeSimulationFunctionalty::operator==(foeSimulationFunctionalty const &rhs) const noexcept {
    return onCreate == rhs.onCreate && onDestroy == rhs.onDestroy &&
           onInitialization == rhs.onInitialization &&
           onDeinitialization == rhs.onDeinitialization &&
           onGfxInitialization == rhs.onGfxInitialization &&
           onGfxDeinitialization == rhs.onGfxDeinitialization;
}

bool foeSimulationFunctionalty::operator!=(foeSimulationFunctionalty const &rhs) const noexcept {
    return !(*this == rhs);
}

auto foeRegisterFunctionality(foeSimulationFunctionalty const &functionality) -> std::error_code {
    std::error_code errC{FOE_SIMULATION_SUCCESS};
    std::scoped_lock lock{mSync};

    if (functionality.id < 1000000000 || functionality.id % 1000 != 0) {
        FOE_LOG(SimulationState, Warning,
                "registerFunctionality - Attempted to register functionality with invalid ID");
        return FOE_SIMULATION_ERROR_FUNCTIONALITY_ID_INVALID;
    }

    for (auto const &it : mRegistered) {
        if (it == functionality) {
            FOE_LOG(SimulationState, Warning,
                    "registerFunctionality - Attempted to re-register functionality");
            return FOE_SIMULATION_ERROR_FUNCTIONALITY_ALREADY_REGISTERED;
        }
        if (it.id == functionality.id) {
            FOE_LOG(SimulationState, Warning,
                    "registerFunctionality - Attempted to register functionality with an ID "
                    "already in use");
            return FOE_SIMULATION_ERROR_FUNCTIONALITY_ID_ALREADY_IN_USE;
        }
    }

    // Not already registered, add it.
    mRegistered.emplace_back(functionality);

    // Go through any already existing SimulationState's and add this new functionality to them.
    foeSimulationState **ppSimState = mStates.data();
    foeSimulationState **ppEndSimState = mStates.data() + mStates.size();
    for (; ppSimState != ppEndSimState; ++ppSimState) {
        acquireExclusiveLock(*ppSimState, "functionality registration");
        if (functionality.onCreate)
            functionality.onCreate(*ppSimState);
        if (foeSimulationIsInitialized(*ppSimState) && functionality.onInitialization) {
            errC = functionality.onInitialization(*ppSimState, &(*ppSimState)->initInfo);
        }
        (*ppSimState)->simSync.unlock();
        if (errC)
            break;
    }

    // If we failed to initialize in one of the simulations above, then we need to go through and
    // deinitialize/destroy this functionality in all the ones it was already added to successfully
    if (errC) {
        --ppSimState;
        ppEndSimState = mStates.data() - 1;
        for (; ppSimState != ppEndSimState; --ppSimState) {
            acquireExclusiveLock(*ppSimState, "functionality deregistration");
            if (functionality.onDeinitialization && foeSimulationIsInitialized(*ppSimState))
                functionality.onDeinitialization(*ppSimState);
            if (functionality.onDestroy)
                functionality.onDestroy(*ppSimState);
            (*ppSimState)->simSync.unlock();
        }
    }

    return errC;
}

auto foeDeregisterFunctionality(foeSimulationFunctionalty const &functionality) -> std::error_code {
    std::scoped_lock lock{mSync};

    for (auto it = mRegistered.begin(); it != mRegistered.end(); ++it) {
        if (*it == functionality) {
            // Since we're deregistering functionality, we need to deinit/destroy this stuff from
            // any active SimulationStates
            for (auto *pSimState : mStates) {
                acquireExclusiveLock(pSimState, "functionality deregistration");
                if (functionality.onDeinitialization && foeSimulationIsInitialized(pSimState))
                    functionality.onDeinitialization(pSimState);
                if (functionality.onDestroy)
                    functionality.onDestroy(pSimState);
                pSimState->simSync.unlock();
            }

            mRegistered.erase(it);
            return FOE_SIMULATION_SUCCESS;
        }
    }

    FOE_LOG(
        SimulationState, Warning,
        "registerFunctionality - Attempted to deregister functionality that was never registered");
    return FOE_SIMULATION_ERROR_FUNCTIONALITY_NOT_REGISTERED;
}

bool foeSimulationIsInitialized(foeSimulationState const *pSimulationState) {
    return !!pSimulationState->initInfo.externalFileSearchFn;
}

bool foeSimulationIsGraphicsInitialzied(foeSimulationState const *pSimulationState) {
    return pSimulationState->gfxSession != FOE_NULL_HANDLE;
}

auto foeCreateSimulation(bool addNameMaps) -> foeSimulationState * {
    std::scoped_lock lock{mSync};

    std::unique_ptr<foeSimulationState> newSimState{new foeSimulationState};

    newSimState->gfxSession = FOE_NULL_HANDLE;

    // Editor Name Maps, if requested
    if (addNameMaps) {
        newSimState->pResourceNameMap = new foeEditorNameMap;
        newSimState->pEntityNameMap = new foeEditorNameMap;
    } else {
        newSimState->pResourceNameMap = nullptr;
        newSimState->pEntityNameMap = nullptr;
    }

    FOE_LOG(SimulationState, Verbose, "Creating SimulationState: {}",
            static_cast<void *>(newSimState.get()));

    // Go through each registered set of functionality, add its items
    for (auto const &registered : mRegistered) {
        if (registered.onCreate) {
            registered.onCreate(newSimState.get());
        }
    }

    // We have the new simulation state, add it to the pool of all states
    mStates.emplace_back(newSimState.get());

    FOE_LOG(SimulationState, Verbose, "Created SimulationState: {}",
            static_cast<void *>(newSimState.get()));

    return newSimState.release();
}

std::error_code foeDestroySimulation(foeSimulationState *pSimulationState) {
    std::scoped_lock lock{mSync};

    FOE_LOG(SimulationState, Verbose, "Destroying SimulationState: {}",
            static_cast<void *>(pSimulationState));

    auto searchIt = std::find(mStates.begin(), mStates.end(), pSimulationState);
    if (searchIt == mStates.end()) {
        // We were given a simulation state that wasn't created from here?
        FOE_LOG(SimulationState, Warning,
                "destroySimulation - Given a SimulationState that wasn't created via "
                "foeCreateSimulation: {}",
                static_cast<void *>(pSimulationState));
        return FOE_SIMULATION_ERROR_SIMULATION_NOT_REGISTERED;
    } else {
        mStates.erase(searchIt);
    }

    // Deinitialize just in case
    if (foeSimulationIsInitialized(pSimulationState))
        deinitSimulation(pSimulationState);

    // Call any destroys
    acquireExclusiveLock(pSimulationState, "simulation destruction");
    for (auto const &registered : mRegistered) {
        if (registered.onDestroy) {
            registered.onDestroy(pSimulationState);
        }
    }
    pSimulationState->simSync.unlock();

    // Destroy Name Maps
    if (pSimulationState->pEntityNameMap)
        delete pSimulationState->pEntityNameMap;
    if (pSimulationState->pResourceNameMap)
        delete pSimulationState->pResourceNameMap;

    // Delete it
    delete pSimulationState;

    FOE_LOG(SimulationState, Verbose, "Destroyed SimulationState: {}",
            static_cast<void *>(pSimulationState));

    return FOE_SIMULATION_SUCCESS;
}

auto foeInitializeSimulation(foeSimulationState *pSimulationState,
                             foeSimulationInitInfo const *pInitInfo) -> std::error_code {
    std::error_code errC;
    std::scoped_lock lock{mSync};

    if (foeSimulationIsInitialized(pSimulationState)) {
        FOE_LOG(SimulationState, Error, "Attempting to re-initialize SimulationState: {}",
                static_cast<void *>(pSimulationState))
        return FOE_SIMULATION_ERROR_ALREADY_INITIALIZED;
    }

    FOE_LOG(SimulationState, Verbose, "Initializing SimulationState: {}",
            static_cast<void *>(pSimulationState));

    pSimulationState->initInfo = *pInitInfo;

    acquireExclusiveLock(pSimulationState, "initialization");
    for (auto const &functionality : mRegistered) {
        if (functionality.onInitialization) {
            errC = functionality.onInitialization(pSimulationState, pInitInfo);
            if (errC)
                break;
        }
    }

    if (errC) {
        FOE_LOG(SimulationState, Error, "Failed to initialize SimulationState: {} due to error: {}",
                static_cast<void *>(pSimulationState), errC.message());
        deinitSimulation(pSimulationState);
    } else {
        FOE_LOG(SimulationState, Verbose, "Initialized SimulationState: {}",
                static_cast<void *>(pSimulationState));
    }

    pSimulationState->simSync.unlock();

    return errC;
}

auto foeDeinitializeSimulation(foeSimulationState *pSimulationState) -> std::error_code {
    std::scoped_lock lock{mSync};

    if (!foeSimulationIsInitialized(pSimulationState))
        return FOE_SIMULATION_ERROR_NOT_INITIALIZED;

    acquireExclusiveLock(pSimulationState, "deinitialization");
    deinitSimulation(pSimulationState);
    return FOE_SIMULATION_SUCCESS;
}

auto foeInitializeSimulationGraphics(foeSimulationState *pSimulationState, foeGfxSession gfxSession)
    -> std::error_code {
    std::error_code errC;
    std::scoped_lock lock{mSync};

    if (foeSimulationIsGraphicsInitialzied(pSimulationState)) {
        FOE_LOG(SimulationState, Error, "Attempting to re-initialize SimulationState graphics: {}",
                static_cast<void *>(pSimulationState))
        return FOE_SIMULATION_ERROR_GRAPHICS_ALREADY_INITIALIZED;
    }

    FOE_LOG(SimulationState, Verbose, "Initializing SimulationState graphics: {}",
            static_cast<void *>(pSimulationState));

    acquireExclusiveLock(pSimulationState, "initializing graphics");

    for (auto const &functionality : mRegistered) {
        if (functionality.onGfxInitialization) {
            errC = functionality.onGfxInitialization(pSimulationState, gfxSession);
            if (errC)
                break;
        }
    }

    if (errC) {
        FOE_LOG(SimulationState, Error,
                "Failed to initialize SimulationState graphics: {} due to error: {}",
                static_cast<void *>(pSimulationState), errC.message());
        deinitializeSimulationGraphics(pSimulationState);
    } else {
        // With success, set the simulation's graphics session handle
        pSimulationState->gfxSession = gfxSession;
        FOE_LOG(SimulationState, Verbose, "Initialized SimulationState graphics: {}",
                static_cast<void *>(pSimulationState));
    }

    pSimulationState->simSync.unlock();

    return FOE_SIMULATION_SUCCESS;
}

auto foeDeinitializeSimulationGraphics(foeSimulationState *pSimulationState) -> std::error_code {
    std::scoped_lock lock{mSync};

    if (!foeSimulationIsGraphicsInitialzied(pSimulationState))
        return FOE_SIMULATION_ERROR_GRAPHICS_NOT_INITIALIZED;

    acquireExclusiveLock(pSimulationState, "deinitializing graphics");
    deinitializeSimulationGraphics(pSimulationState);
    return FOE_SIMULATION_SUCCESS;
}