// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef VERTEX_DESCRIPTOR_H
#define VERTEX_DESCRIPTOR_H

#include <foe/imex/binary/exporter.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

foeResultSet export_foeVertexDescriptorCreateInfo(foeResourceCreateInfo createInfo,
                                                  foeImexBinarySet *pBinarySet,
                                                  foeImexBinaryFiles *pFiles);

#ifdef __cplusplus
}
#endif

#endif // VERTEX_DESCRIPTOR_H