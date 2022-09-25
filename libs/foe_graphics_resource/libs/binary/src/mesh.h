// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MESH_H
#define MESH_H

#include <foe/imex/binary/exporter.h>
#include <foe/imex/binary/importer.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

foeResultSet export_foeMeshFileCreateInfo(foeResourceCreateInfo createInfo,
                                          foeImexBinarySet *pBinarySet,
                                          foeImexBinaryFiles *pFiles);

foeResultSet import_foeMeshFileCreateInfo(void const *pReadBuffer,
                                          uint32_t *pReadSize,
                                          foeEcsGroupTranslator groupTranslator,
                                          foeResourceCreateInfo *pResourceCI);

foeResultSet create_foeMeshFileCreateInfo(foeResourceID resource,
                                          foeResourceCreateInfo resourceCI,
                                          foeSimulation const *pSimulation);

foeResultSet export_foeMeshCubeCreateInfo(foeResourceCreateInfo createInfo,
                                          foeImexBinarySet *pBinarySet,
                                          foeImexBinaryFiles *pFiles);

foeResultSet import_foeMeshCubeCreateInfo(void const *pReadBuffer,
                                          uint32_t *pReadSize,
                                          foeEcsGroupTranslator groupTranslator,
                                          foeResourceCreateInfo *pResourceCI);

foeResultSet create_foeMeshCubeCreateInfo(foeResourceID resource,
                                          foeResourceCreateInfo resourceCI,
                                          foeSimulation const *pSimulation);

foeResultSet export_foeMeshIcosphereCreateInfo(foeResourceCreateInfo createInfo,
                                               foeImexBinarySet *pBinarySet,
                                               foeImexBinaryFiles *pFiles);

foeResultSet import_foeMeshIcosphereCreateInfo(void const *pReadBuffer,
                                               uint32_t *pReadSize,
                                               foeEcsGroupTranslator groupTranslator,
                                               foeResourceCreateInfo *pResourceCI);

foeResultSet create_foeMeshIcosphereCreateInfo(foeResourceID resource,
                                               foeResourceCreateInfo resourceCI,
                                               foeSimulation const *pSimulation);

#ifdef __cplusplus
}
#endif

#endif // MESH_H