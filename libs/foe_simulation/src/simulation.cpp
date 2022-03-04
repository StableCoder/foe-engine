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
    auto const endIt = mRegistered.rend();
    for (auto it = mRegistered.rbegin(); it != endIt; ++it) {
        if (it->onDeinitialization) {
            it->onDeinitialization(pSimulationState);
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
    auto const endIt = mRegistered.rend();
    for (auto it = mRegistered.rbegin(); it != endIt; ++it) {
        if (it->onGfxDeinitialization) {
            it->onGfxDeinitialization(pSimulationState);
        }
    }

    pSimulationState->gfxSession = FOE_NULL_HANDLE;

    pSimulationState->simSync.unlock();

    FOE_LOG(SimulationState, Verbose, "Deinitialized SimulationState Graphics: {}",
            static_cast<void *>(pSimulationState));
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
    foeSimulationState **ppSimState = mStates.data();
    foeSimulationState **ppEndSimState = mStates.data() + mStates.size();

    for (; ppSimState != ppEndSimState; ++ppSimState) {
        // Reset the set of passed calls
        passedStates = {};

        acquireExclusiveLock(*ppSimState, "functionality registration");

        if (functionality.onCreate) {
            errC = functionality.onCreate(*ppSimState);
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
        if (foeSimulationIsInitialized(*ppSimState) && functionality.onInitialization) {
            errC = functionality.onInitialization(*ppSimState, &(*ppSimState)->initInfo);
            if (errC) {
                FOE_LOG(SimulationState, Error,
                        "foeRegisterFunctionality - Failed initializing functionality on "
                        "SimulationState: {} due to error: {}",
                        static_cast<void *>(*ppSimState), errC.message());
                break;
            }
            passedStates.initialized = true;
        }
        if (foeSimulationIsGraphicsInitialzied(*ppSimState) && functionality.onGfxInitialization) {
            errC = functionality.onGfxInitialization(*ppSimState, (*ppSimState)->gfxSession);
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
        if (passedStates.gfxInitialized && functionality.onGfxDeinitialization &&
            foeSimulationIsGraphicsInitialzied(*ppSimState)) {
            functionality.onGfxDeinitialization(*ppSimState);
        }
        if (passedStates.initialized && functionality.onDeinitialization &&
            foeSimulationIsInitialized(*ppSimState)) {
            functionality.onDeinitialization(*ppSimState);
        }
        if (passedStates.created && functionality.onDestroy) {
            functionality.onDeinitialization(*ppSimState);
        }

        (*ppSimState)->simSync.unlock();

        // Go through the rest of the simulations
        --ppSimState;
        ppEndSimState = mStates.data() - 1;
        for (; ppSimState != ppEndSimState; --ppSimState) {
            acquireExclusiveLock(*ppSimState, "functionality deregistration");

            if (functionality.onGfxDeinitialization &&
                foeSimulationIsGraphicsInitialzied(*ppSimState)) {
                functionality.onGfxDeinitialization(*ppSimState);
            }

            if (functionality.onDeinitialization && foeSimulationIsInitialized(*ppSimState)) {
                functionality.onDeinitialization(*ppSimState);
            }

            if (functionality.onDestroy) {
                functionality.onDestroy(*ppSimState);
            }
            (*ppSimState)->simSync.unlock();
        }

        return errC;
    }

    return FOE_SIMULATION_SUCCESS;
}

auto foeDeregisterFunctionality(foeSimulationFunctionalty const &functionality) -> std::error_code {
    std::scoped_lock lock{mSync};

    for (auto it = mRegistered.begin(); it != mRegistered.end(); ++it) {
        if (it->id == functionality.id) {
            // Since we're deregistering functionality, we need to deinit/destroy this stuff from
            // any active SimulationStates
            for (auto *pSimState : mStates) {
                acquireExclusiveLock(pSimState, "functionality deregistration");
                if (functionality.onGfxDeinitialization &&
                    foeSimulationIsGraphicsInitialzied(pSimState)) {
                    functionality.onGfxDeinitialization(pSimState);
                }

                if (functionality.onDeinitialization && foeSimulationIsInitialized(pSimState)) {
                    functionality.onDeinitialization(pSimState);
                }

                if (functionality.onDestroy) {
                    functionality.onDestroy(pSimState);
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

bool foeSimulationIsInitialized(foeSimulationState const *pSimulationState) {
    return !!pSimulationState->initInfo.externalFileSearchFn;
}

bool foeSimulationIsGraphicsInitialzied(foeSimulationState const *pSimulationState) {
    return pSimulationState->gfxSession != FOE_NULL_HANDLE;
}

auto foeCreateSimulation(bool addNameMaps, foeSimulationState **ppSimulationState)
    -> std::error_code {
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
    std::error_code errC{FOE_SIMULATION_SUCCESS};
    auto it = mRegistered.begin();
    auto const endIt = mRegistered.end();
    for (; it != endIt; ++it) {
        if (it->onCreate) {
            errC = it->onCreate(newSimState.get());
            if (errC) {
                FOE_LOG(SimulationState, Error,
                        "Error creating SimulationState: {} due to error: {}",
                        static_cast<void *>(newSimState.get()), errC.message());
                break;
            }
        }
    }

    // If there is an error code at this point, then we need to walk-back what was created so far
    if (errC && it != mRegistered.begin()) {
        --it;
        for (; it >= mRegistered.begin(); --it) {
            it->onDestroy(newSimState.get());
        }
    }

    if (!errC) {
        FOE_LOG(SimulationState, Verbose, "Created SimulationState: {}",
                static_cast<void *>(newSimState.get()));
        mStates.emplace_back(newSimState.get());
        *ppSimulationState = newSimState.release();
    }

    return errC;
}

auto foeDestroySimulation(foeSimulationState *pSimulationState) -> std::error_code {
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
        return FOE_SIMULATION_ERROR_NOT_REGISTERED;
    } else {
        mStates.erase(searchIt);
    }

    acquireExclusiveLock(pSimulationState, "simulation destruction");

    if (foeSimulationIsInitialized(pSimulationState))
        deinitSimulation(pSimulationState);

    auto const endIt = mRegistered.rend();
    for (auto it = mRegistered.rbegin(); it != endIt; ++it) {
        if (it->onDestroy) {
            it->onDestroy(pSimulationState);
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
    std::scoped_lock lock{mSync};

    if (foeSimulationIsInitialized(pSimulationState)) {
        FOE_LOG(SimulationState, Error, "Attempting to re-initialize SimulationState: {}",
                static_cast<void *>(pSimulationState))
        return FOE_SIMULATION_ERROR_SIMULATION_ALREADY_INITIALIZED;
    }

    FOE_LOG(SimulationState, Verbose, "Initializing SimulationState: {}",
            static_cast<void *>(pSimulationState));

    acquireExclusiveLock(pSimulationState, "initialization");

    // Go through each set of functionality and call the initialization function, if one is attached
    std::error_code errC{FOE_SIMULATION_SUCCESS};
    auto it = mRegistered.begin();
    auto const endIt = mRegistered.end();

    for (; it != endIt; ++it) {
        if (it->onInitialization) {
            errC = it->onInitialization(pSimulationState, pInitInfo);
            if (errC) {
                FOE_LOG(SimulationState, Error,
                        "Failed to initialize SimulationState: {} due to error: {}",
                        static_cast<void *>(pSimulationState), errC.message());
                break;
            }
        }
    }

    // If there is an error code at this point, then we need to walk-back what was created so far
    if (errC && it != mRegistered.begin()) {
        --it;
        for (; it >= mRegistered.begin(); --it) {
            if (it->onDeinitialization) {
                it->onDeinitialization(pSimulationState);
            }
        }
    }

    if (!errC) {
        // On a successful initialization, set it into the state itself
        pSimulationState->initInfo = *pInitInfo;
        FOE_LOG(SimulationState, Verbose, "Initialized SimulationState: {}",
                static_cast<void *>(pSimulationState));
    }

    pSimulationState->simSync.unlock();

    return errC;
}

auto foeDeinitializeSimulation(foeSimulationState *pSimulationState) -> std::error_code {
    std::scoped_lock lock{mSync};

    if (!foeSimulationIsInitialized(pSimulationState))
        return FOE_SIMULATION_ERROR_SIMULATION_NOT_INITIALIZED;

    acquireExclusiveLock(pSimulationState, "deinitialization");
    deinitSimulation(pSimulationState);
    return FOE_SIMULATION_SUCCESS;
}

auto foeInitializeSimulationGraphics(foeSimulationState *pSimulationState, foeGfxSession gfxSession)
    -> std::error_code {
    std::scoped_lock lock{mSync};

    if (foeSimulationIsGraphicsInitialzied(pSimulationState)) {
        FOE_LOG(SimulationState, Error, "Attempting to re-initialize SimulationState graphics: {}",
                static_cast<void *>(pSimulationState))
        return FOE_SIMULATION_ERROR_SIMULATION_GRAPHICS_ALREADY_INITIALIZED;
    }

    FOE_LOG(SimulationState, Verbose, "Initializing SimulationState graphics: {}",
            static_cast<void *>(pSimulationState));

    acquireExclusiveLock(pSimulationState, "initializing graphics");

    // Go through each set of functionality and call the initialization function, if one is attached
    std::error_code errC{FOE_SIMULATION_SUCCESS};
    auto it = mRegistered.begin();
    auto const endIt = mRegistered.end();

    for (; it != endIt; ++it) {
        if (it->onGfxInitialization) {
            errC = it->onGfxInitialization(pSimulationState, gfxSession);
            if (errC) {
                FOE_LOG(SimulationState, Error,
                        "Failed to initialize SimulationState graphics: {} due to error: {}",
                        static_cast<void *>(pSimulationState), errC.message());
                break;
            }
        }
    }

    // If there is an error code at this point, then we need to walk-back what was created so far
    if (errC && it != mRegistered.begin()) {
        --it;
        for (; it >= mRegistered.begin(); --it) {
            if (it->onGfxDeinitialization) {
                it->onGfxDeinitialization(pSimulationState);
            }
        }
    }

    if (!errC) {
        // With success, set the simulation's graphics session handle
        pSimulationState->gfxSession = gfxSession;
        FOE_LOG(SimulationState, Verbose, "Initialized SimulationState graphics: {}",
                static_cast<void *>(pSimulationState));
    }

    pSimulationState->simSync.unlock();

    return errC;
}

auto foeDeinitializeSimulationGraphics(foeSimulationState *pSimulationState) -> std::error_code {
    std::scoped_lock lock{mSync};

    if (!foeSimulationIsGraphicsInitialzied(pSimulationState))
        return FOE_SIMULATION_ERROR_SIMULATION_GRAPHICS_NOT_INITIALIZED;

    acquireExclusiveLock(pSimulationState, "deinitializing graphics");
    deinitializeSimulationGraphics(pSimulationState);
    return FOE_SIMULATION_SUCCESS;
}