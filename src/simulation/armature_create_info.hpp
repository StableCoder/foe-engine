// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef ARMATURE_CREATE_INFO_HPP
#define ARMATURE_CREATE_INFO_HPP

#include <foe/resource/create_info.h>

#include <stdint.h>

struct AnimationImportInfo {
    char const *pFile;
    char const *pName;
};

struct foeArmatureCreateInfo {
    char const *pFile;
    char const *pRootArmatureNode;
    uint32_t animationCount;
    AnimationImportInfo *pAnimations;
};

void cleanup_AnimationImportInfo(AnimationImportInfo *pData);

void cleanup_foeArmatureCreateInfo(foeArmatureCreateInfo *pData);

void foeDestroyArmatureCreateInfo(foeResourceCreateInfoType type, void *pCreateInfo);

#endif // ARMATURE_CREATE_INFO_HPP