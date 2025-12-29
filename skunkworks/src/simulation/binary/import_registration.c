// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "import_registration.h"

#include <foe/imex/binary/importer.h>

#include "../binary.h"
#include "armature_create_info.h"
#include "armature_state.h"
#include "render_state.h"
#include "result.h"

foeResultSet foeSkunkworksBinaryRegisterImporters() {
    foeResultSet result = to_foeResult(FOE_SKUNKWORKS_BINARY_SUCCESS);

    // Resources
    result = foeImexBinaryRegisterResourceImportFns(binary_key_foeArmatureCreateInfo(),
                                                    import_foeArmatureCreateInfo);
    if (result.value != FOE_SUCCESS)
        goto REGISTRATION_FAILED;

    // Components
    result = foeImexBinaryRegisterComponentImportFn(binary_key_foeArmatureState(),
                                                    import_foeArmatureState);
    if (result.value != FOE_SUCCESS)
        goto REGISTRATION_FAILED;

    result =
        foeImexBinaryRegisterComponentImportFn(binary_key_foeRenderState(), import_foeRenderState);
    if (result.value != FOE_SUCCESS)
        goto REGISTRATION_FAILED;

REGISTRATION_FAILED:
    if (result.value != FOE_SUCCESS)
        foeSkunkworksBinaryDeregisterImporters();

    return result;
}

void foeSkunkworksBinaryDeregisterImporters() {
    // Components
    foeImexBinaryDeregisterComponentImportFn(binary_key_foeRenderState(), import_foeRenderState);
    foeImexBinaryDeregisterComponentImportFn(binary_key_foeArmatureState(),
                                             import_foeArmatureState);

    // Resources
    foeImexBinaryDeregisterResourceImportFns(binary_key_foeArmatureCreateInfo(),
                                             import_foeArmatureCreateInfo);
}