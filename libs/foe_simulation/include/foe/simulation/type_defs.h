// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_SIMULATION_TYPE_DEFS_H
#define FOE_SIMULATION_TYPE_DEFS_H

#include <foe/simulation/export.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foeSimulation foeSimulation;

typedef int foeSimulationStructureType;

struct foeSimulationBaseStruct {
    foeSimulationStructureType sType;
    void *pNext;
};

FOE_SIM_EXPORT
void *foeSimulationGetResourceLoader(foeSimulation const *pSimulation,
                                     foeSimulationStructureType sType);

FOE_SIM_EXPORT
void *foeSimulationGetSystem(foeSimulation const *pSimulation, foeSimulationStructureType sType);

FOE_SIM_EXPORT
void *foeSimulationGetComponentPool(foeSimulation const *pSimulation,
                                    foeSimulationStructureType sType);

#ifdef __cplusplus
}
#endif

#endif // FOE_SIMULATION_TYPE_DEFS_H