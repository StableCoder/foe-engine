// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef SHADER_H
#define SHADER_H

#include <foe/imex/binary/exporter.h>
#include <foe/imex/binary/importer.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

foeResultSet export_foeShaderCreateInfo(foeResourceCreateInfo createInfo,
                                        foeImexBinarySet *pBinarySet,
                                        foeImexBinaryFiles *pFiles);

foeResultSet import_foeShaderCreateInfo(void const *pReadBuffer,
                                        uint32_t *pReadSize,
                                        foeEcsGroupTranslator groupTranslator,
                                        foeResourceCreateInfo *pResourceCI);

#ifdef __cplusplus
}
#endif

#endif // SHADER_H