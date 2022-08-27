// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "compare.h"

#include "armature_create_info.h"

#include <string.h>

bool compare_AnimationImportInfo(AnimationImportInfo const *pData1,
                                 AnimationImportInfo const *pData2) {
    // char const * - pFile[null-terminated]
    if (pData1->pFile != pData2->pFile || pData1->pFile == NULL || pData2->pFile == NULL ||
        strcmp(pData1->pFile, pData2->pFile) != 0) {
        return false;
    }

    // char const * - pName[null-terminated]
    if (pData1->pName != pData2->pName || pData1->pName == NULL || pData2->pName == NULL ||
        strcmp(pData1->pName, pData2->pName) != 0) {
        return false;
    }

    return true;
}

bool compare_foeArmatureCreateInfo(foeArmatureCreateInfo const *pData1,
                                   foeArmatureCreateInfo const *pData2) {
    // uint32_t - animationCount
    if (pData1->animationCount != pData2->animationCount) {
        return false;
    }

    // char const * - pFile[null-terminated]
    if (pData1->pFile != pData2->pFile || pData1->pFile == NULL || pData2->pFile == NULL ||
        strcmp(pData1->pFile, pData2->pFile) != 0) {
        return false;
    }

    // char const * - pRootArmatureNode[null-terminated]
    if (pData1->pRootArmatureNode != pData2->pRootArmatureNode ||
        pData1->pRootArmatureNode == NULL || pData2->pRootArmatureNode == NULL ||
        strcmp(pData1->pRootArmatureNode, pData2->pRootArmatureNode) != 0) {
        return false;
    }

    // AnimationImportInfo* - pAnimations[animationCount]
    if (pData1->pAnimations != pData2->pAnimations) {
        if (pData1->pAnimations == NULL || pData2->pAnimations == NULL) {
            return false;
        }

        for (size_t i = 0; i < pData1->animationCount; ++i) {
            if (!compare_AnimationImportInfo(pData1->pAnimations + i, pData2->pAnimations + i)) {
                return false;
            }
        }
    }

    return true;
}
