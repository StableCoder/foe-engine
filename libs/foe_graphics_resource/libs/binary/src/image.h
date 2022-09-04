// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IMAGE_H
#define IMAGE_H

#include <foe/imex/binary/exporter.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

foeResultSet export_foeImageCreateInfo(foeResourceCreateInfo createInfo,
                                       foeImexBinarySet *pBinarySet,
                                       foeImexBinaryFiles *pFiles);

#ifdef __cplusplus
}
#endif

#endif // IMAGE_H