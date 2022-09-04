// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MESH_H
#define MESH_H

#include <foe/imex/binary/exporter.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

foeResultSet export_foeMeshFileCreateInfo(foeResourceCreateInfo createInfo,
                                          foeImexBinarySet *pBinarySet,
                                          foeImexBinaryFiles *pFiles);

foeResultSet export_foeMeshCubeCreateInfo(foeResourceCreateInfo createInfo,
                                          foeImexBinarySet *pBinarySet,
                                          foeImexBinaryFiles *pFiles);

foeResultSet export_foeMeshIcosphereCreateInfo(foeResourceCreateInfo createInfo,
                                               foeImexBinarySet *pBinarySet,
                                               foeImexBinaryFiles *pFiles);

#ifdef __cplusplus
}
#endif

#endif // MESH_H