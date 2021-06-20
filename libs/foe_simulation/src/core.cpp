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

#include <foe/simulation/core.hpp>

#include <foe/ecs/editor_name_map.hpp>
#include <foe/simulation/error_code.hpp>
#include <foe/simulation/state.hpp>

#include "log.hpp"

#include <mutex>
#include <vector>

namespace {
std::mutex mSync;

std::vector<foeSimulationFunctionalty> mRegistered;
std::vector<foeSimulationState *> mStates;

void deinitSimulation(foeSimulationState *pSimulationState) {
    FOE_LOG(SimulationState, Verbose, "Deinitializing SimulationState: {}",
            static_cast<void *>(pSimulationState));

    for (auto const &functionality : mRegistered) {
        if (functionality.onDeinitialization) {
            functionality.onDeinitialization(pSimulationState);
        }
    }

    FOE_LOG(SimulationState, Verbose, "Deinitialized SimulationState: {}",
            static_cast<void *>(pSimulationState));
}

foeSimulationStateLists createSimStateLists(foeSimulationState *pSimulationState) {
    return foeSimulationStateLists{
        .pResourcePools = pSimulationState->resourcePools.data(),
        .resourcePoolCount = static_cast<uint32_t>(pSimulationState->resourcePools.size()),
        .pResourceLoaders = pSimulationState->resourceLoaders.data(),
        .resourceLoaderCount = static_cast<uint32_t>(pSimulationState->resourceLoaders.size()),
        .pComponentPools = pSimulationState->componentPools.data(),
        .componentPoolCount = static_cast<uint32_t>(pSimulationState->componentPools.size()),
        .pSystems = pSimulationState->systems.data(),
        .systemCount = static_cast<uint32_t>(pSimulationState->systems.size()),
    };
}

} // namespace

bool foeSimulationFunctionalty::operator==(foeSimulationFunctionalty const &rhs) const noexcept {
    return onCreate == rhs.onCreate && onDestroy == rhs.onDestroy &&
           onInitialization == rhs.onInitialization && onDeinitialization == rhs.onDeinitialization;
}

bool foeSimulationFunctionalty::operator!=(foeSimulationFunctionalty const &rhs) const noexcept {
    return !(*this == rhs);
}

auto foeRegisterFunctionality(foeSimulationFunctionalty const &functionality) -> std::error_code {
    std::scoped_lock lock{mSync};

    for (auto const &it : mRegistered) {
        if (it == functionality) {
            FOE_LOG(SimulationState, Warning,
                    "registerFunctionality - Attempted to re-register functionality");
            return FOE_SIMULATION_ERROR_FUNCTIONALITY_ALREADY_REGISTERED;
        }
    }

    // Not already registered, add it.
    mRegistered.emplace_back(functionality);

    // Go through any already existing SimulationState's and add this new functionality to them.
    for (auto *pSimState : mStates) {
        if (functionality.onCreate)
            functionality.onCreate(pSimState);
        if (foeSimulationIsInitialized(pSimState) && functionality.onInitialization) {
            auto simStateLists = createSimStateLists(pSimState);
            functionality.onInitialization(&pSimState->initInfo, &simStateLists);
        }
    }

    return FOE_SIMULATION_SUCCESS;
}

auto foeDeregisterFunctionality(foeSimulationFunctionalty const &functionality) -> std::error_code {
    std::scoped_lock lock{mSync};

    for (auto it = mRegistered.begin(); it != mRegistered.end(); ++it) {
        if (*it == functionality) {
            // Since we're deregistering functionality, we need to deinit/destroy this stuff from
            // any active SimulationStates
            for (auto *pSimState : mStates) {
                if (functionality.onDeinitialization)
                    functionality.onDeinitialization(pSimState);
                if (functionality.onDestroy)
                    functionality.onDestroy(pSimState);
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

foeSimulationState *foeCreateSimulation(bool addNameMaps) {
    std::scoped_lock lock{mSync};

    std::unique_ptr<foeSimulationState> newSimState{new foeSimulationState};

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
    deinitSimulation(pSimulationState);

    // Call any destroys
    for (auto const &registered : mRegistered) {
        if (registered.onDestroy) {
            registered.onDestroy(pSimulationState);
        }
    }

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

void foeInitializeSimulation(foeSimulationState *pSimulationState,
                             foeSimulationInitInfo const *pInitInfo) {
    std::scoped_lock lock{mSync};

    if (pSimulationState->initInfo.gfxSession != FOE_NULL_HANDLE) {
        FOE_LOG(SimulationState, Error,
                "Attempting to re-initialize already initialized SimulationState: {}",
                static_cast<void *>(pSimulationState))
        return;
    }

    FOE_LOG(SimulationState, Verbose, "Initializing SimulationState: {}",
            static_cast<void *>(pSimulationState));

    pSimulationState->initInfo = *pInitInfo;

    auto stateLists = createSimStateLists(pSimulationState);

    for (auto const &functionality : mRegistered) {
        if (functionality.onInitialization) {
            functionality.onInitialization(pInitInfo, &stateLists);
        }
    }

    FOE_LOG(SimulationState, Verbose, "Initialized SimulationState: {}",
            static_cast<void *>(pSimulationState));
}

void foeDeinitializeSimulation(foeSimulationState *pSimulationState) {
    std::scoped_lock lock{mSync};

    deinitSimulation(pSimulationState);
}