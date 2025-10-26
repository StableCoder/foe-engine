// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef ANIMATED_BONE_STATE_HPP
#define ANIMATED_BONE_STATE_HPP

#include <foe/ecs/component_pool.h>
#include <foe/resource/resource.h>
#include <glm/glm.hpp>

#include "type_defs.h"

typedef foeEcsComponentPool foeAnimatedBoneStatePool;

struct foeAnimatedBoneState {
    foeResource armature;

    uint32_t boneCount{0};
    glm::mat4 *pBones{nullptr};
};

void cleanup_foeAnimatedBoneState(foeAnimatedBoneState const *pData);

#endif // ANIMATED_BONE_STATE_HPP