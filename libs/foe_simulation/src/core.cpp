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

#include <foe/simulation/state.hpp>

#include "log.hpp"

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

} // namespace

bool foeRegisterFunctionality(foeSimulationFunctionalty const &functionality) {
    std::scoped_lock lock{mSync};

    for (auto const &it : mRegistered) {
        if (it.onCreate == functionality.onCreate && it.onDestroy == functionality.onDestroy) {
            FOE_LOG(SimulationState, Warning,
                    "registerFunctionality - Attempted to re-register functionality");
            return false;
        }
    }

    // Not already registered, add it.
    mRegistered.emplace_back(functionality);

    return true;
}

void foeDeregisterFunctionality(foeSimulationFunctionalty const &functionality) {
    std::scoped_lock lock{mSync};

    for (auto it = mRegistered.begin(); it != mRegistered.end(); ++it)
        if (it->onCreate == functionality.onCreate && it->onDestroy == functionality.onDestroy) {
            mRegistered.erase(it);
            return;
        }

    FOE_LOG(
        SimulationState, Warning,
        "registerFunctionality - Attempted to deregister functionality that was never registered");
}

foeSimulationState *foeCreateSimulation() {
    std::scoped_lock lock{mSync};

    std::unique_ptr<foeSimulationState> newSimState{new foeSimulationState};
    newSimState->pResourceNameMap = nullptr;
    newSimState->pEntityNameMap = nullptr;

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

void foeDestroySimulation(foeSimulationState *pSimulationState) {
    std::scoped_lock lock{mSync};

    FOE_LOG(SimulationState, Verbose, "Destroying SimulationState: {}",
            static_cast<void *>(pSimulationState));

    // Deinitialize just in case
    deinitSimulation(pSimulationState);

    // Call any destroys
    for (auto const &registered : mRegistered) {
        if (registered.onDestroy) {
            registered.onDestroy(pSimulationState);
        }
    }

    // Delete it anyways
    delete pSimulationState;

    auto searchIt = std::find(mStates.begin(), mStates.end(), pSimulationState);
    if (searchIt == mStates.end()) {
        // We were given a simulation state that wasn't created from here?
        FOE_LOG(SimulationState, Warning,
                "destroySimulation - Given a SimulationState that wasn't created via a "
                "foeCreateSimulation");
    } else {
        mStates.erase(searchIt);
    }

    FOE_LOG(SimulationState, Verbose, "Destroyed SimulationState: {}",
            static_cast<void *>(pSimulationState));
}

void foeInitializeSimulation(foeSimulationState *pSimulationState,
                             foeSimulationInitInfo pInitInfo) {
    std::scoped_lock lock{mSync};

    FOE_LOG(SimulationState, Verbose, "Initializing SimulationState: {}",
            static_cast<void *>(pSimulationState));

    pInitInfo.pResourcePools = pSimulationState->resourcePools.data();
    pInitInfo.resourcePoolCount = pSimulationState->resourcePools.size();
    pInitInfo.pResourceLoaders = pSimulationState->resourceLoaders.data();
    pInitInfo.resourceLoaderCount = pSimulationState->resourceLoaders.size();
    pInitInfo.pComponentPools = pSimulationState->componentPools.data();
    pInitInfo.componentPoolCount = pSimulationState->componentPools.size();
    pInitInfo.pSystems = pSimulationState->systems.data();
    pInitInfo.systemCount = pSimulationState->systems.size();

    for (auto const &functionality : mRegistered) {
        if (functionality.onInitialization) {
            functionality.onInitialization(&pInitInfo);
        }
    }

    FOE_LOG(SimulationState, Verbose, "Initialized SimulationState: {}",
            static_cast<void *>(pSimulationState));
}

void foeDeinitializeSimulation(foeSimulationState *pSimulationState) {
    std::scoped_lock lock{mSync};

    deinitSimulation(pSimulationState);
}