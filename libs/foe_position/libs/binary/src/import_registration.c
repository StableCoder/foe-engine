// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/position/binary/import_registration.h>

#include <foe/imex/binary/importer.h>
#include <foe/position/binary.h>

#include "position_3d.h"
#include "result.h"

foeResultSet foePositionBinaryRegisterImporters() {
    foeResultSet result = to_foeResult(FOE_POSITION_BINARY_SUCCESS);

    // Components
    result =
        foeImexBinaryRegisterComponentImportFn(binary_key_foePosition3d(), import_foePosition3D);
    if (result.value != FOE_SUCCESS)
        goto REGISTRATION_FAILED;

REGISTRATION_FAILED:
    if (result.value != FOE_SUCCESS)
        foePositionBinaryDeregisterImporters();

    return result;
}

void foePositionBinaryDeregisterImporters() {
    // Components
    foeImexBinaryDeregisterComponentImportFn(binary_key_foePosition3d(), import_foePosition3D);
}