// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/position/binary/export_registration.h>

#include <foe/imex/binary/exporter.h>
#include <foe/imex/exporters.h>
#include <foe/position/binary.h>
#include <foe/position/type_defs.h>

#include "position_3d.h"
#include "result.h"

#include <stdlib.h>
#include <string.h>

static void onDeregister(foeExporter exporter) {
    if (strcmp(exporter.pName, "Binary") == 0) {
        // Components
        foeImexBinaryDeregisterComponentExportFn(export_foePosition3D);
    }
}

static foeResultSet onRegister(foeExporter exporter) {
    if (strcmp(exporter.pName, "Binary")) {
        // Components
        foeResultSet result = foeImexBinaryRegisterComponentExportFn(export_foePosition3D);
        if (result.value != FOE_SUCCESS)
            return result;
    }

    return to_foeResult(FOE_POSITION_BINARY_SUCCESS);
}

static foeExportFunctionality const cExportFunctionality = {
    .onRegister = onRegister,
    .onDeregister = onDeregister,
};

foeResultSet foePositionBinaryRegisterExporters() {
    return foeRegisterExportFunctionality(&cExportFunctionality);
}

void foePositionBinaryDeregisterExporters() {
    foeDeregisterExportFunctionality(&cExportFunctionality);
}