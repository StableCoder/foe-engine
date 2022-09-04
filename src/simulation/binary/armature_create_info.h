// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef SIMULATION_BINARY_ARMATURE_CREATE_INFO_H
#define SIMULATION_BINARY_ARMATURE_CREATE_INFO_H

#include <foe/imex/binary/exporter.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

foeResultSet export_foeArmatureCreateInfo(foeResourceCreateInfo createInfo,
                                          foeImexBinarySet *pBinarySet,
                                          foeImexBinaryFiles *pFiles);

#ifdef __cplusplus
}
#endif

#endif // SIMULATION_BINARY_ARMATURE_CREATE_INFO_H