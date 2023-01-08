// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_PHYSICS_COMPONENT_RIGID_BODY_H
#define FOE_PHYSICS_COMPONENT_RIGID_BODY_H

#include <foe/ecs/id.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef class btRigidBody btRigidBody;

struct foeRigidBody {
    btRigidBody *pRigidBody;
    float mass;
    foeResourceID collisionShape;
};

#ifdef __cplusplus
}
#endif

#endif // FOE_PHYSICS_COMPONENT_RIGID_BODY_H