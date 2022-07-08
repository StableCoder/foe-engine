// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_PHYSICS_COMPONENT_RIGID_BODY_POOL_HPP
#define FOE_PHYSICS_COMPONENT_RIGID_BODY_POOL_HPP

#include <foe/data_pool.hpp>
#include <foe/ecs/id.h>
#include <foe/physics/component/rigid_body.hpp>
#include <foe/physics/type_defs.h>

class foeRigidBodyPool : public foeDataPool<foeEntityID, foeRigidBody> {
  public:
    void maintenance() { foeDataPool<foeEntityID, foeRigidBody>::maintenance(); }
};

#endif // FOE_PHYSICS_COMPONENT_RIGID_BODY_POOL_HPP