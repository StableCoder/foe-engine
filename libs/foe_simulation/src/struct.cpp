/*
    Copyright (C) 2022 George Cave.

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

#include <foe/simulation/type_defs.h>

#include <foe/simulation/core/component_pool_base.hpp>
#include <foe/simulation/core/pool.hpp>
#include <foe/simulation/core/system.hpp>
#include <foe/simulation/simulation.hpp>

extern "C" void *foeSimulationGetResourcePool(foeSimulationState const *pSimulationState,
                                              foeSimulationStructureType sType) {
    auto *pIt = pSimulationState->resourcePools.data();
    auto *pEndIt = pIt + pSimulationState->resourcePools.size();

    for (; pIt != pEndIt; ++pIt) {
        if (*pIt != nullptr && (*pIt)->sType == sType) {
            return (void *)*pIt;
        }
    }

    return nullptr;
}

extern "C" void *foeSimulationGetResourceLoader(foeSimulationState const *pSimulationState,
                                                foeSimulationStructureType sType) {
    auto *pIt = pSimulationState->resourceLoaders.data();
    auto *pEndIt = pIt + pSimulationState->resourceLoaders.size();

    for (; pIt != pEndIt; ++pIt) {
        if (pIt->sType == sType) {
            return (void *)pIt->pLoader;
        }
    }

    return nullptr;
}

extern "C" void *foeSimulationGetSystem(foeSimulationState const *pSimulationState,
                                        foeSimulationStructureType sType) {
    auto *pIt = pSimulationState->systems.data();
    auto *pEndIt = pIt + pSimulationState->systems.size();

    for (; pIt != pEndIt; ++pIt) {
        if (*pIt != nullptr && (*pIt)->sType == sType) {
            return (void *)*pIt;
        }
    }

    return nullptr;
}

extern "C" void *foeSimulationGetComponentPool(foeSimulationState const *pSimulationState,
                                               foeSimulationStructureType sType) {
    auto *pIt = pSimulationState->componentPools.data();
    auto *pEndIt = pIt + pSimulationState->componentPools.size();

    for (; pIt != pEndIt; ++pIt) {
        if (*pIt != nullptr && (*pIt)->sType == sType) {
            return (void *)*pIt;
        }
    }

    return nullptr;
}