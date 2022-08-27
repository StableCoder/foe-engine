// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_PHYSICS_COMPARE_H
#define FOE_PHYSICS_COMPARE_H

#include <foe/physics/export.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foeRigidBody foeRigidBody;
typedef struct foeCollisionShapeCreateInfo foeCollisionShapeCreateInfo;

FOE_PHYSICS_EXPORT bool compare_foeRigidBody(foeRigidBody const *pData1,
                                             foeRigidBody const *pData2);

FOE_PHYSICS_EXPORT bool compare_foeCollisionShapeCreateInfo(
    foeCollisionShapeCreateInfo const *pData1, foeCollisionShapeCreateInfo const *pData2);

#ifdef __cplusplus
}
#endif

#endif // FOE_POSITION_COMPARE_H
