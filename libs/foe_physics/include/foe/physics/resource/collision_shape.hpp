// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_HPP
#define FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_HPP

#include <btBulletDynamicsCommon.h>

#include <memory>

struct foeCollisionShape {
    std::unique_ptr<btCollisionShape> collisionShape;
};

#endif // FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_HPP