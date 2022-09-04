// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/imex/binary/exporter.h>

#include <foe/imex/exporters.h>

#include "exporter.h"

static foeExporter const cExporter = {
    .pName = "Binary",
    .version = FOE_EXPORTER_VERSION(0, 0, 0),
    .pExportFn = foeImexBinaryExport,
};

foeResultSet foeImexBinaryRegisterExporter() { return foeImexRegisterExporter(cExporter); }

void foeImexBinaryDeregisterExporter() { foeImexDeregisterExporter(cExporter); }