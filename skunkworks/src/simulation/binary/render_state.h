// Copyright (C) 2022-2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef SIMULATION_BINARY_RENDER_STATE_H
#define SIMULATION_BINARY_RENDER_STATE_H

#include <foe/imex/binary/exporter.h>
#include <foe/imex/binary/importer.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeSimulation)

foeResultSet export_foeRenderState(foeEntityID entity,
                                   foeSimulation simulation,
                                   foeImexBinarySet *pBinarySet);

foeResultSet import_foeRenderState(void const *pReadBuffer,
                                   uint32_t *pReadSize,
                                   foeEcsGroupTranslator groupTranslator,
                                   foeEntityID entity,
                                   foeSimulation simulation);

#ifdef __cplusplus
}
#endif

#endif // SIMULATION_BINARY_RENDER_STATE_H