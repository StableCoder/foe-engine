// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/error_code.h>

#ifndef EXPORTER_HPP
#define EXPORTER_HPP

struct foeSimulation;

foeResultSet foeImexYamlExport(char const *pExportPath, foeSimulation *pSimState);

#endif // EXPORTER_HPP