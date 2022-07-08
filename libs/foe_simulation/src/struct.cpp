// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/simulation/type_defs.h>

#include <foe/simulation/simulation.hpp>

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