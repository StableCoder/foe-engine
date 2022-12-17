// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "export_registration.h"

#include <foe/imex/binary/exporter.h>
#include <foe/imex/exporters.h>
#include <foe/resource/create_info.h>
#include <foe/resource/pool.h>

#include "armature_create_info.h"
#include "armature_state.h"
#include "render_state.h"
#include "result.h"

#include <stdlib.h>
#include <string.h>

static void onDeregister(foeExporter exporter) {
    if (strcmp(exporter.pName, "Yaml") == 0) {
        // Component
        foeImexBinaryDeregisterComponentExportFn(export_foeRenderState);
        foeImexBinaryDeregisterComponentExportFn(export_foeArmatureState);

        // Resource
        foeImexBinaryDeregisterResourceExportFn(export_foeArmatureCreateInfo);
    }
}

static foeResultSet onRegister(foeExporter exporter) {
    foeResultSet result = to_foeResult(FOE_BRINGUP_BINARY_SUCCESS);

    if (strcmp(exporter.pName, "Binary") == 0) {
        // Resource
        result = foeImexBinaryRegisterResourceExportFn(export_foeArmatureCreateInfo);
        if (result.value != FOE_SUCCESS) {
            goto REGISTRATION_FAILED;
        }

        // Component
        result = foeImexBinaryRegisterComponentExportFn(export_foeArmatureState);
        if (result.value != FOE_SUCCESS) {
            goto REGISTRATION_FAILED;
        }

        result = foeImexBinaryRegisterComponentExportFn(export_foeRenderState);
        if (result.value != FOE_SUCCESS) {
            goto REGISTRATION_FAILED;
        }
    }

REGISTRATION_FAILED:
    if (result.value != FOE_SUCCESS)
        onDeregister(exporter);

    return result;
}

static foeExportFunctionality const cExportFunctionality = {
    .onRegister = onRegister,
    .onDeregister = onDeregister,
};

foeResultSet foeBringupBinaryRegisterExporters() {
    return foeRegisterExportFunctionality(&cExportFunctionality);
}

void foeBringupBinaryDeregisterExporters() {
    foeDeregisterExportFunctionality(&cExportFunctionality);
}