// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "armature_create_info.h"

#include <stdlib.h>

void foeCleanup_AnimationImportInfo(AnimationImportInfo *pData) {
    if (pData->pName)
        free((char *)pData->pName);
    if (pData->pFile)
        free((char *)pData->pFile);
}

void foeCleanup_foeArmatureCreateInfo(foeArmatureCreateInfo *pData) {
    if (pData->pAnimations) {
        for (uint32_t i = 0; i < pData->animationCount; ++i) {
            foeCleanup_AnimationImportInfo(&pData->pAnimations[i]);
        }
        free(pData->pAnimations);
    }

    if (pData->pRootArmatureNode)
        free((char *)pData->pRootArmatureNode);
    if (pData->pFile)
        free((char *)pData->pFile);
}
