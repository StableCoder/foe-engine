// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef ARMATURE_SYSTEM_HPP
#define ARMATURE_SYSTEM_HPP

#include <foe/resource/pool.h>

#include "armature_state_pool.hpp"

#include <vector>

class foeArmatureSystem {
  public:
    foeResultSet initialize(foeResourcePool resourcePool,
                            foeArmatureStatePool armatureStatePool,
                            foeAnimatedBoneStatePool animatedBoneStatePool);
    void deinitialize();
    bool initialized() const noexcept;

    foeResultSet process(float timePassed);

  private:
    // Resources
    foeResourcePool mResourcePool{FOE_NULL_HANDLE};

    // Components
    foeArmatureStatePool mArmatureStatePool{FOE_NULL_HANDLE};
    foeAnimatedBoneStatePool mAnimatedBoneStatePool{FOE_NULL_HANDLE};

    struct AwaitingData {
        foeEntityID entity;
        foeResource armature;
    };

    std::vector<AwaitingData> mAwaitingLoading;
};

#endif // ARMATURE_SYSTEM_HPP