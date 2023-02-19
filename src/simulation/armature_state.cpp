// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "armature_state.hpp"

void cleanup_foeAnimatedBoneState(foeAnimatedBoneState const *pData) {
    assert(pData->armature != FOE_NULL_HANDLE);

    foeResourceDecrementUseCount(pData->armature);
    foeResourceDecrementRefCount(pData->armature);

    free(pData->pBones);
}