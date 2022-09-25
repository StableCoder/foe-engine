// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATERIAL_H
#define MATERIAL_H

#include <foe/imex/binary/exporter.h>
#include <foe/imex/binary/importer.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

foeResultSet export_foeMaterialCreateInfo(foeResourceCreateInfo createInfo,
                                          foeImexBinarySet *pBinarySet,
                                          foeImexBinaryFiles *pFiles);

foeResultSet import_foeMaterialCreateInfo(void const *pReadBuffer,
                                          uint32_t *pReadSize,
                                          foeEcsGroupTranslator groupTranslator,
                                          foeResourceCreateInfo *pResourceCI);

foeResultSet create_foeMaterialCreateInfo(foeResourceID resource,
                                          foeResourceCreateInfo resourceCI,
                                          foeSimulation const *pSimulation);

#ifdef __cplusplus
}
#endif

#endif // MATERIAL_H