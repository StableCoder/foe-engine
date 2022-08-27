// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "cleanup.h"

#include "armature_create_info.h"

#include <stddef.h>
#include <stdlib.h>

void cleanup_AnimationImportInfo(AnimationImportInfo *pData) {
    // char const * - pName
    if (pData->pName) {
        free((char *)pData->pName);
    }

    // char const * - pFile
    if (pData->pFile) {
        free((char *)pData->pFile);
    }
}

void cleanup_foeArmatureCreateInfo(foeArmatureCreateInfo *pData) {
    // AnimationImportInfo* - pAnimations
    if (pData->pAnimations) {
        for (size_t i = 0; i < pData->animationCount; ++i) {
            cleanup_AnimationImportInfo(pData->pAnimations + i);
        }
        free((AnimationImportInfo *)pData->pAnimations);
    }

    // char const * - pRootArmatureNode
    if (pData->pRootArmatureNode) {
        free((char *)pData->pRootArmatureNode);
    }

    // char const * - pFile
    if (pData->pFile) {
        free((char *)pData->pFile);
    }
}
