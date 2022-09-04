// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/physics/binary/export_registration.h>

#include <foe/imex/binary/exporter.h>
#include <foe/imex/exporters.h>

#include "collision_shape.h"
#include "result.h"
#include "rigid_body.h"

#include <stdlib.h>
#include <string.h>

static void onDeregister(foeExporter exporter) {
    if (strcmp(exporter.pName, "Binary") == 0) {
        // Components
        foeImexBinaryDeregisterComponentExportFn(export_foeRigidBody);

        // Resources
        foeImexBinaryDeregisterResourceExportFn(export_foeCollisionShapeCreateInfo);
    }
}

static foeResultSet onRegister(foeExporter exporter) {
    if (strcmp(exporter.pName, "Binary")) {
        // Resources
        foeResultSet result =
            foeImexBinaryRegisterResourceExportFn(export_foeCollisionShapeCreateInfo);
        if (result.value != FOE_SUCCESS)
            return result;

        // Components
        result = foeImexBinaryRegisterComponentExportFn(export_foeRigidBody);
        if (result.value != FOE_SUCCESS)
            return result;
    }

    return to_foeResult(FOE_PHYSICS_BINARY_SUCCESS);
}

static foeExportFunctionality const cExportFunctionality = {
    .onRegister = onRegister,
    .onDeregister = onDeregister,
};

foeResultSet foePhysicsBinaryRegisterExporters() {
    return foeRegisterExportFunctionality(&cExportFunctionality);
}

void foePhysicsBinaryDeregisterExporters() {
    foeDeregisterExportFunctionality(&cExportFunctionality);
}