// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef ARMATURE_SYSTEM_HPP
#define ARMATURE_SYSTEM_HPP

#include <foe/resource/pool.h>

#include "armature_state_pool.hpp"

class foeArmatureSystem {
  public:
    foeResultSet initialize(foeResourcePool resourcePool, foeArmatureStatePool armatureStatePool);
    void deinitialize();
    bool initialized() const noexcept;

    foeResultSet process(float timePassed);

  private:
    // Resources
    foeResourcePool mResourcePool{FOE_NULL_HANDLE};

    // Components
    foeArmatureStatePool mArmatureStatePool{FOE_NULL_HANDLE};

    foeEcsEntityList modifiedEntityList{FOE_NULL_HANDLE};
};

#endif // ARMATURE_SYSTEM_HPP