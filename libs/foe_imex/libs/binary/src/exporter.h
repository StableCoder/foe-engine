// Copyright (C) 2022-2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef EXPORTER_H
#define EXPORTER_H

#include <foe/handle.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeSimulation)

foeResultSet foeImexBinaryExport(char const *pExportPath, foeSimulation simulation);

#ifdef __cplusplus
}
#endif

#endif // EXPORTER_H