// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef ARMATURE_SYSTEM_HPP
#define ARMATURE_SYSTEM_HPP

#include <foe/resource/pool.h>

class foeArmatureStatePool;

class foeArmatureSystem {
  public:
    foeResultSet initialize(foeResourcePool resourcePool, foeArmatureStatePool *pArmatureStatePool);
    void deinitialize();
    bool initialized() const noexcept;

    foeResultSet process(float timePassed);

  private:
    // Resources
    foeResourcePool mResourcePool{FOE_NULL_HANDLE};

    // Components
    foeArmatureStatePool *mpArmatureStatePool{nullptr};
};

#endif // ARMATURE_SYSTEM_HPP