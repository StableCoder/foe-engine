// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_PHYSICS_COMPONENT_RIGID_BODY_H
#define FOE_PHYSICS_COMPONENT_RIGID_BODY_H

#include <foe/ecs/id.h>

typedef class btRigidBody btRigidBody;

struct foeRigidBody {
    btRigidBody *pRigidBody;
    float mass;
    foeResourceID collisionShape;
};

#endif // FOE_PHYSICS_COMPONENT_RIGID_BODY_H