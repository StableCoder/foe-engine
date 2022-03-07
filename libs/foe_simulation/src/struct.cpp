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

#include <foe/simulation/simulation.hpp>

extern "C" void *foeSimulationGetResourcePool(foeSimulation const *pSimulation,
                                              foeSimulationStructureType sType) {
    auto *pIt = pSimulation->resourcePools.data();
    auto *pEndIt = pIt + pSimulation->resourcePools.size();

    for (; pIt != pEndIt; ++pIt) {
        if (pIt->sType == sType) {
            return (void *)pIt->pResourcePool;
        }
    }

    return nullptr;
}

extern "C" void *foeSimulationGetResourceLoader(foeSimulation const *pSimulation,
                                                foeSimulationStructureType sType) {
    auto *pIt = pSimulation->resourceLoaders.data();
    auto *pEndIt = pIt + pSimulation->resourceLoaders.size();

    for (; pIt != pEndIt; ++pIt) {
        if (pIt->sType == sType) {
            return pIt->pLoader;
        }
    }

    return nullptr;
}

extern "C" void *foeSimulationGetSystem(foeSimulation const *pSimulation,
                                        foeSimulationStructureType sType) {
    auto *pIt = pSimulation->systems.data();
    auto *pEndIt = pIt + pSimulation->systems.size();

    for (; pIt != pEndIt; ++pIt) {
        if (pIt->sType == sType) {
            return pIt->pSystem;
        }
    }

    return nullptr;
}

extern "C" void *foeSimulationGetComponentPool(foeSimulation const *pSimulation,
                                               foeSimulationStructureType sType) {
    auto *pIt = pSimulation->componentPools.data();
    auto *pEndIt = pIt + pSimulation->componentPools.size();

    for (; pIt != pEndIt; ++pIt) {
        if (pIt->sType == sType) {
            return pIt->pComponentPool;
        }
    }

    return nullptr;
}