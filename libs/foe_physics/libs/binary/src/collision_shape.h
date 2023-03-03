// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef COLLISION_SHAPE_CREATE_INFO_H
#define COLLISION_SHAPE_CREATE_INFO_H

#include <foe/imex/binary/exporter.h>
#include <foe/imex/binary/importer.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

foeResultSet export_foeCollisionShapeCreateInfo(foeResourceCreateInfo createInfo,
                                                foeImexBinarySet *pBinarySet,
                                                foeImexBinaryFiles *pFiles);

foeResultSet import_foeCollisionShapeCreateInfo(void const *pReadBuffer,
                                                uint32_t *pReadSize,
                                                foeEcsGroupTranslator groupTranslator,
                                                foeResourceCreateInfo *pResourceCI);

#ifdef __cplusplus
}
#endif

#endif // COLLISION_SHAPE_CREATE_INFO_H