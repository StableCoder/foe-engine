// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/binary/export_registration.h>

#include <foe/imex/binary/exporter.h>
#include <foe/imex/exporters.h>

#include "image.h"
#include "material.h"
#include "mesh.h"
#include "result.h"
#include "shader.h"
#include "vertex_descriptor.h"

#include <stdlib.h>
#include <string.h>

static void onDeregister(foeExporter exporter) {
    if (strcmp(exporter.pName, "Binary") == 0) {
        // Resources
        foeImexBinaryDeregisterResourceExportFn(export_foeVertexDescriptorCreateInfo);
        foeImexBinaryDeregisterResourceExportFn(export_foeShaderCreateInfo);
        foeImexBinaryDeregisterResourceExportFn(export_foeMeshIcosphereCreateInfo);
        foeImexBinaryDeregisterResourceExportFn(export_foeMeshCubeCreateInfo);
        foeImexBinaryDeregisterResourceExportFn(export_foeMeshFileCreateInfo);
        foeImexBinaryDeregisterResourceExportFn(export_foeMaterialCreateInfo);
        foeImexBinaryDeregisterResourceExportFn(export_foeImageCreateInfo);
    }
}

static foeResultSet onRegister(foeExporter exporter) {
    if (strcmp(exporter.pName, "Binary")) {
        // Resources
        foeResultSet result = foeImexBinaryRegisterResourceExportFn(export_foeImageCreateInfo);
        if (result.value != FOE_SUCCESS)
            return result;

        result = foeImexBinaryRegisterResourceExportFn(export_foeMaterialCreateInfo);
        if (result.value != FOE_SUCCESS)
            return result;

        result = foeImexBinaryRegisterResourceExportFn(export_foeMeshFileCreateInfo);
        if (result.value != FOE_SUCCESS)
            return result;

        result = foeImexBinaryRegisterResourceExportFn(export_foeMeshCubeCreateInfo);
        if (result.value != FOE_SUCCESS)
            return result;

        result = foeImexBinaryRegisterResourceExportFn(export_foeMeshIcosphereCreateInfo);
        if (result.value != FOE_SUCCESS)
            return result;

        result = foeImexBinaryRegisterResourceExportFn(export_foeShaderCreateInfo);
        if (result.value != FOE_SUCCESS)
            return result;

        result = foeImexBinaryRegisterResourceExportFn(export_foeVertexDescriptorCreateInfo);
        if (result.value != FOE_SUCCESS)
            return result;
    }

    return to_foeResult(FOE_GRAPHICS_RESOURCE_BINARY_SUCCESS);
}

static foeExportFunctionality const cExportFunctionality = {
    .onRegister = onRegister,
    .onDeregister = onDeregister,
};

foeResultSet foeGraphicsResourceBinaryRegisterExporters() {
    return foeRegisterExportFunctionality(&cExportFunctionality);
}

void foeGraphicsResourceBinaryDeregisterExporters() {
    foeDeregisterExportFunctionality(&cExportFunctionality);
}