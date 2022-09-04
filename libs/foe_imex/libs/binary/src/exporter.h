// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef EXPORTER_H
#define EXPORTER_H

#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foeSimulation foeSimulation;

foeResultSet foeImexBinaryExport(char const *pExportPath, foeSimulation *pSimState);

#ifdef __cplusplus
}
#endif

#endif // EXPORTER_H