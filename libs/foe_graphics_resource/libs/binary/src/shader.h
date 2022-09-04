// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef SHADER_H
#define SHADER_H

#include <foe/imex/binary/exporter.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

foeResultSet export_foeShaderCreateInfo(foeResourceCreateInfo createInfo,
                                        foeImexBinarySet *pBinarySet,
                                        foeImexBinaryFiles *pFiles);

#ifdef __cplusplus
}
#endif

#endif // SHADER_H