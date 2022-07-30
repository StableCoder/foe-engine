// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_PHYSICS_COMPONENT_RIGID_BODY_HPP
#define FOE_PHYSICS_COMPONENT_RIGID_BODY_HPP

#include <btBulletDynamicsCommon.h>
#include <foe/ecs/id.h>

struct foeRigidBody {
    btRigidBody *pRigidBody;
    float mass;
    foeResourceID collisionShape;
};

#endif // FOE_PHYSICS_COMPONENT_RIGID_BODY_HPP