// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/physics/binary/import_registration.h>

#include <foe/imex/binary/importer.h>
#include <foe/physics/binary.h>

#include "collision_shape.h"
#include "result.h"
#include "rigid_body.h"

foeResultSet foePhysicsBinaryRegisterImporters() {
    foeResultSet result = to_foeResult(FOE_PHYSICS_BINARY_SUCCESS);

    // Resources
    result = foeImexBinaryRegisterResourceImportFns(binary_key_foeCollisionShapeCreateInfo(),
                                                    import_foeCollisionShapeCreateInfo,
                                                    create_foeCollisionShapeCreateInfo);
    if (result.value != FOE_SUCCESS)
        goto REGISTRATION_FAILED;

    // Components
    result = foeImexBinaryRegisterComponentImportFn(binary_key_foeRigidBody(), import_foeRigidBody);
    if (result.value != FOE_SUCCESS)
        goto REGISTRATION_FAILED;

REGISTRATION_FAILED:
    if (result.value != FOE_SUCCESS)
        foePhysicsBinaryDeregisterImporters();

    return result;
}

void foePhysicsBinaryDeregisterImporters() {
    // Components
    foeImexBinaryDeregisterComponentImportFn(binary_key_foeRigidBody(), import_foeRigidBody);

    // Resources
    foeImexBinaryDeregisterResourceImportFns(binary_key_foeCollisionShapeCreateInfo(),
                                             import_foeCollisionShapeCreateInfo,
                                             create_foeCollisionShapeCreateInfo);
}