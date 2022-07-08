// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_POSITION_COMPONENT_3D_POOL_HPP
#define FOE_POSITION_COMPONENT_3D_POOL_HPP

#include <foe/data_pool.hpp>
#include <foe/ecs/id.h>
#include <foe/position/component/3d.hpp>
#include <foe/position/type_defs.h>

#include <memory>

class foePosition3dPool : public foeDataPool<foeEntityID, std::unique_ptr<foePosition3d>> {
  public:
    void maintenance() { foeDataPool<foeEntityID, std::unique_ptr<foePosition3d>>::maintenance(); }
};

#endif // FOE_POSITION_COMPONENT_3D_POOL_HPP