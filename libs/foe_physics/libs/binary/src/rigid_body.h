// Copyright (C) 2022-2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RIGID_BODY_H
#define RIGID_BODY_H

#include <foe/imex/binary/exporter.h>
#include <foe/imex/binary/importer.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

foeResultSet export_foeRigidBody(foeEntityID entity,
                                 foeSimulation simulation,
                                 foeImexBinarySet *pBinarySets);

foeResultSet import_foeRigidBody(void const *pReadBuffer,
                                 uint32_t *pReadSize,
                                 foeEcsGroupTranslator groupTranslator,
                                 foeEntityID entity,
                                 foeSimulation simulation);

#ifdef __cplusplus
}
#endif

#endif // RIGID_BODY_H