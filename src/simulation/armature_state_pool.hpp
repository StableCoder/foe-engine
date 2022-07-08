// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef ARMATURE_STATE_POOL_HPP
#define ARMATURE_STATE_POOL_HPP

#include <foe/data_pool.hpp>

#include "armature_state.hpp"
#include "type_defs.h"

class foeArmatureStatePool : public foeDataPool<foeEntityID, foeArmatureState> {
  public:
    void maintenance() { foeDataPool<foeEntityID, foeArmatureState>::maintenance(); }
};

#endif // ARMATURE_STATE_POOL_HPP