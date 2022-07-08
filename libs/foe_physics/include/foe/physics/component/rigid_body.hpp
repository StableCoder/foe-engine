// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_PHYSICS_COMPONENT_RIGID_BODY_HPP
#define FOE_PHYSICS_COMPONENT_RIGID_BODY_HPP

#include <btBulletDynamicsCommon.h>
#include <foe/ecs/id.h>

#include <memory>

struct foeRigidBody {
    std::unique_ptr<btRigidBody> rigidBody;
    float mass;
    foeResourceID collisionShape;
};

#endif // FOE_PHYSICS_COMPONENT_RIGID_BODY_HPP