// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "armature_create_info.hpp"

void cleanup_AnimationImportInfo(AnimationImportInfo *pData) {
    if (pData->pAnimationNames)
        delete[] pData->pAnimationNames;
}

void foeDestroyArmatureCreateInfo(foeResourceCreateInfoType type, void *pCreateInfo) {
    auto *pCI = (foeArmatureCreateInfo *)pCreateInfo;

    if (pCI->pAnimationSets) {
        for (uint32_t i = 0; i < pCI->animationSetCount; ++i) {
            cleanup_AnimationImportInfo(&pCI->pAnimationSets[i]);
        }
        delete[] pCI->pAnimationSets;
    }

    pCI->~foeArmatureCreateInfo();
}
