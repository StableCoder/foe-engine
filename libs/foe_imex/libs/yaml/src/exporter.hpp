// Copyright (C) 2021-2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef EXPORTER_HPP
#define EXPORTER_HPP

#include <foe/handle.h>
#include <foe/result.h>

FOE_DEFINE_HANDLE(foeSimulation)

foeResultSet foeImexYamlExport(char const *pExportPath, foeSimulation simulation);

#endif // EXPORTER_HPP