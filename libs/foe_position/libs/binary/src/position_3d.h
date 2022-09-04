// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef POSITION_3D_H
#define POSITION_3D_H

#include <foe/imex/binary/exporter.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

foeResultSet export_foePosition3D(foeEntityID entity,
                                  foeSimulation const *pSimulation,
                                  foeImexBinarySet *pBinarySets);

#ifdef __cplusplus
}
#endif

#endif // POSITION_3D_H