// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "armature_create_info.hpp"

void cleanup_AnimationImportInfo(AnimationImportInfo *pData) {}

void foeDestroyArmatureCreateInfo(foeResourceCreateInfoType type, void *pCreateInfo) {
    auto *pCI = (foeArmatureCreateInfo *)pCreateInfo;

    if (pCI->pAnimations) {
        for (uint32_t i = 0; i < pCI->animationCount; ++i) {
            cleanup_AnimationImportInfo(&pCI->pAnimations[i]);
        }
        delete[] pCI->pAnimations;
    }

    pCI->~foeArmatureCreateInfo();
}
