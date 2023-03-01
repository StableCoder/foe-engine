// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "animated_bone_state.hpp"

void cleanup_foeAnimatedBoneState(foeAnimatedBoneState const *pData) {
    if (pData->armature) {
        foeResourceDecrementUseCount(pData->armature);
        foeResourceDecrementRefCount(pData->armature);
    }

    free(pData->pBones);
}