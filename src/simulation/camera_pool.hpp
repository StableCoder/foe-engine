// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef CAMERA_POOL_HPP
#define CAMERA_POOL_HPP

#include <foe/data_pool.hpp>
#include <foe/ecs/id.h>

#include "camera.hpp"
#include "type_defs.h"

#include <memory>

class foeCameraPool : public foeDataPool<foeEntityID, std::unique_ptr<Camera>> {
  public:
    void maintenance() { foeDataPool<foeEntityID, std::unique_ptr<Camera>>::maintenance(); }
};

#endif // CAMERA_POOL_HPP