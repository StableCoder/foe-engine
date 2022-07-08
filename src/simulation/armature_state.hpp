// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef ARMATURE_STATE_HPP
#define ARMATURE_STATE_HPP

#include <foe/ecs/id.h>
#include <glm/glm.hpp>

#include <vector>

struct foeArmatureState {
    // Armature information
    foeId armatureID{FOE_INVALID_ID};
    std::vector<glm::mat4> armatureState;

    // Animation info
    uint32_t animationID{UINT32_MAX};
    float time{0.f};
};

#endif // ARMATURE_STATE_HPP