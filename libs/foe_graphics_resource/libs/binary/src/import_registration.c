// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/binary/import_registration.h>

#include <foe/graphics/resource/binary.h>
#include <foe/imex/binary/importer.h>

#include "image.h"
#include "material.h"
#include "mesh.h"
#include "result.h"
#include "shader.h"
#include "vertex_descriptor.h"

#include <stdlib.h>
#include <string.h>

foeResultSet foeGraphicsResourceBinaryRegisterImporters() {
    foeResultSet result = to_foeResult(FOE_GRAPHICS_RESOURCE_BINARY_SUCCESS);

    // Resources
    result = foeImexBinaryRegisterResourceImportFns(binary_key_foeImageCreateInfo(),
                                                    import_foeImageCreateInfo);
    if (result.value != FOE_SUCCESS)
        goto REGISTRATION_FAILED;

    result = foeImexBinaryRegisterResourceImportFns(binary_key_foeMaterialCreateInfo(),
                                                    import_foeMaterialCreateInfo);
    if (result.value != FOE_SUCCESS)
        goto REGISTRATION_FAILED;

    result = foeImexBinaryRegisterResourceImportFns(binary_key_foeMeshFileCreateInfo(),
                                                    import_foeMeshFileCreateInfo);
    if (result.value != FOE_SUCCESS)
        goto REGISTRATION_FAILED;

    result = foeImexBinaryRegisterResourceImportFns(binary_key_foeMeshCubeCreateInfo(),
                                                    import_foeMeshCubeCreateInfo);
    if (result.value != FOE_SUCCESS)
        goto REGISTRATION_FAILED;

    result = foeImexBinaryRegisterResourceImportFns(binary_key_foeMeshIcosphereCreateInfo(),
                                                    import_foeMeshIcosphereCreateInfo);
    if (result.value != FOE_SUCCESS)
        goto REGISTRATION_FAILED;

    result = foeImexBinaryRegisterResourceImportFns(binary_key_foeShaderCreateInfo(),
                                                    import_foeShaderCreateInfo);
    if (result.value != FOE_SUCCESS)
        goto REGISTRATION_FAILED;

    result = foeImexBinaryRegisterResourceImportFns(binary_key_foeVertexDescriptorCreateInfo(),
                                                    import_foeVertexDescriptorCreateInfo);
    if (result.value != FOE_SUCCESS)
        goto REGISTRATION_FAILED;

REGISTRATION_FAILED:
    if (result.value != FOE_SUCCESS)
        foeGraphicsResourceBinaryDeregisterImporters();

    return result;
}

void foeGraphicsResourceBinaryDeregisterImporters() {
    // Resources
    foeImexBinaryDeregisterResourceImportFns(binary_key_foeVertexDescriptorCreateInfo(),
                                             import_foeVertexDescriptorCreateInfo);

    foeImexBinaryDeregisterResourceImportFns(binary_key_foeShaderCreateInfo(),
                                             import_foeShaderCreateInfo);

    foeImexBinaryDeregisterResourceImportFns(binary_key_foeMeshIcosphereCreateInfo(),
                                             import_foeMeshIcosphereCreateInfo);

    foeImexBinaryDeregisterResourceImportFns(binary_key_foeMeshCubeCreateInfo(),
                                             import_foeMeshCubeCreateInfo);

    foeImexBinaryDeregisterResourceImportFns(binary_key_foeMeshFileCreateInfo(),
                                             import_foeMeshFileCreateInfo);

    foeImexBinaryDeregisterResourceImportFns(binary_key_foeMaterialCreateInfo(),
                                             import_foeMaterialCreateInfo);

    foeImexBinaryDeregisterResourceImportFns(binary_key_foeImageCreateInfo(),
                                             import_foeImageCreateInfo);
}