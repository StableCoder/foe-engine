// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef ARMATURE_STATE_HPP
#define ARMATURE_STATE_HPP

#include <foe/ecs/id.h>
#include <glm/glm.hpp>

struct foeArmatureState {
    // Armature information
    foeId armatureID{FOE_INVALID_ID};
    uint32_t armatureBoneCount{0};
    glm::mat4 *pArmatureBones{nullptr};

    // Animation info
    uint32_t animationID{UINT32_MAX};
    float time{0.f};

    ~foeArmatureState() { free(pArmatureBones); }
};

#endif // ARMATURE_STATE_HPP