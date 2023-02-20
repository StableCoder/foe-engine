// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef ANIMATED_BONE_SYSTEM_H
#define ANIMATED_BONE_SYSTEM_H

#include <foe/handle.h>
#include <foe/resource/pool.h>
#include <foe/result.h>

#include "animated_bone_state_pool.h"
#include "armature_state_pool.h"

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeAnimatedBoneSystem)

foeResultSet foeCreateAnimatedBoneSystem(foeAnimatedBoneSystem *pAnimatedBoneSystem);

void foeDestroyAnimatedBoneSystem(foeAnimatedBoneSystem animatedBoneSystem);

foeResultSet foeInitializeAnimatedBoneSystem(foeAnimatedBoneSystem animatedBoneSystem,
                                             foeResourcePool resourcePool,
                                             foeArmatureStatePool armatureStatePool,
                                             foeAnimatedBoneStatePool animatedBoneStatePool);

void foeDeinitializeAnimatedBoneSystem(foeAnimatedBoneSystem animatedBoneSystem);

foeResultSet foeProcessAnimatedBoneSystem(foeAnimatedBoneSystem animatedBoneSystem,
                                          float timeElapsed);

#ifdef __cplusplus
}
#endif

#endif // ANIMATED_BONE_SYSTEM_H