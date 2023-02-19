// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef ARMATURE_STATE_HPP
#define ARMATURE_STATE_HPP

#include <foe/ecs/id.h>

struct foeArmatureState {
    // Armature information
    foeId armatureID{FOE_INVALID_ID};
    // Animation info
    uint32_t animationID{UINT32_MAX};
    float time{0.f};
};

#include <foe/resource/resource.h>
#include <glm/glm.hpp>

struct foeAnimatedBoneState {
    foeResource armature;

    uint32_t boneCount{0};
    glm::mat4 *pBones{nullptr};
};

void cleanup_foeAnimatedBoneState(foeAnimatedBoneState const *pData);

#endif // ARMATURE_STATE_HPP