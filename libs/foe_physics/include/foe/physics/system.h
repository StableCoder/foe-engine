// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_PHYSICS_SYSTEM_H
#define FOE_PHYSICS_SYSTEM_H

#include <foe/handle.h>
#include <foe/physics/component/rigid_body_pool.h>
#include <foe/physics/export.h>
#include <foe/position/component/3d_pool.h>
#include <foe/resource/pool.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foePhysicsSystem)

FOE_PHYSICS_EXPORT
foeResultSet foePhysicsCreateSystem(foePhysicsSystem *pPhysicsSystem);

FOE_PHYSICS_EXPORT
void foePhysicsDestroySystem(foePhysicsSystem physicsSystem);

FOE_PHYSICS_EXPORT
foeResultSet foePhysicsInitializeSystem(foePhysicsSystem physicsSystem,
                                        foeResourcePool resourcePool,
                                        foeRigidBodyPool rigidBodyPool,
                                        foePosition3dPool positionPool);

FOE_PHYSICS_EXPORT
void foePhysicsDeinitializeSystem(foePhysicsSystem physicsSystem);

FOE_PHYSICS_EXPORT
foeResultSet foePhysicsProcessSystem(foePhysicsSystem physicsSystem, float timeElapsed);

#ifdef __cplusplus
}
#endif

#endif // FOE_PHYSICS_SYSTEM_H