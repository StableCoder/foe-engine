// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef ARMATURE_STATE_H
#define ARMATURE_STATE_H

#include <foe/ecs/component_pool.h>
#include <foe/ecs/id.h>

#include "type_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef foeEcsComponentPool foeArmatureStatePool;

struct foeArmatureState {
    // Armature information
    foeId armatureID{FOE_INVALID_ID};
    // Animation info
    uint32_t animationID{UINT32_MAX};
    float time{0.f};
};

#ifdef __cplusplus
}
#endif

#endif // ARMATURE_STATE_H