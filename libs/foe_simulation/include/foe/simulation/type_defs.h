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

/**
 * This macro is used to help diferentiate different binaries/plugin's functionality set.
 *
 * For the C-interface to work, some enum values, such as 'structure types' need to be unique across
 * all disparate compilations, but this cannot be easily assured normally. This handy macro, when
 * given a value between 0 and 1.000.000 will return an ID which should be the basis for enum block
 *of values which should be considered reserved for that functionality to use.
 *
 * For example, if AUD_FUN calls FOE_SIMULATION_FUNCTIONALITY_ID(1024), then AUD_FUN should should
 * consider itself to have free reign to use values 1.001.024.000 to 1.001.024.999. This AUD_FUN
 *value should then be passed when the functionality is registered, so that it can be determined if
 *this value is shared with any other set of loaded functionality, and be rejected if clashes could
 * occur.
 *
 * If functionality is rejected for this reason, then choosing a different value (there are 990.000
 * to choose from).
 *
 * @warning 0 is reserved for the main application and should not be used for non-application
 *functionality
 * @warning 1 - 9.999 is reserved for FoE-developed functionality and should not be used by others.
 **/
#define FOE_SIMULATION_FUNCTIONALITY_ID(X) 1000000000 + (X * 1000)

typedef int foeSimulationStructureType;

struct foeSimulationBaseStruct {
    foeSimulationStructureType sType;
    void *pNext;
};

FOE_SIM_EXPORT void *foeSimulationGetResourceLoader(foeSimulation const *pSimulation,
                                                    foeSimulationStructureType sType);

FOE_SIM_EXPORT void *foeSimulationGetSystem(foeSimulation const *pSimulation,
                                            foeSimulationStructureType sType);

FOE_SIM_EXPORT void *foeSimulationGetComponentPool(foeSimulation const *pSimulation,
                                                   foeSimulationStructureType sType);

#ifdef __cplusplus
}
#endif

#endif // FOE_SIMULATION_TYPE_DEFS_H