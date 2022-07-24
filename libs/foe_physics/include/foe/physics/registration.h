// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_PHYSICS_REGISTRATION_H
#define FOE_PHYSICS_REGISTRATION_H

#include <foe/error_code.h>
#include <foe/physics/export.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_PHYSICS_EXPORT int foePhysicsFunctionalityID();

FOE_PHYSICS_EXPORT foeResultSet foePhysicsRegisterFunctionality();

FOE_PHYSICS_EXPORT void foePhysicsDeregisterFunctionality();

#ifdef __cplusplus
}
#endif

#endif // FOE_PHYSICS_REGISTRATION_H