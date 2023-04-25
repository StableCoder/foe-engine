// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_PHYSICS_BINARY_IMPORT_REGISTRATION_HPP
#define FOE_PHYSICS_BINARY_IMPORT_REGISTRATION_HPP

#include <foe/physics/binary/export.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_PHYSICS_BINARY_EXPORT
foeResultSet foePhysicsBinaryRegisterImporters();

FOE_PHYSICS_BINARY_EXPORT
void foePhysicsBinaryDeregisterImporters();

#ifdef __cplusplus
}
#endif

#endif // FOE_PHYSICS_BINARY_IMPORT_REGISTRATION_HPP