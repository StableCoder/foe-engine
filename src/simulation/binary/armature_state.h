// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef SIMULATION_BINARY_ARMATURE_STATE_H
#define SIMULATION_BINARY_ARMATURE_STATE_H

#include <foe/imex/binary/exporter.h>
#include <foe/imex/binary/importer.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foeSimulation foeSimulation;

foeResultSet export_foeArmatureState(foeEntityID entity,
                                     foeSimulation const *pSimulation,
                                     foeImexBinarySet *pBinarySet);

foeResultSet import_foeArmatureState(void const *pReadBuffer,
                                     uint32_t *pReadSize,
                                     foeEcsGroupTranslator groupTranslator,
                                     foeEntityID entity,
                                     foeSimulation const *pSimulation);

#ifdef __cplusplus
}
#endif

#endif // SIMULATION_BINARY_ARMATURE_STATE_H