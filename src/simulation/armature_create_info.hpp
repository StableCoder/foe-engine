// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef ARMATURE_CREATE_INFO_HPP
#define ARMATURE_CREATE_INFO_HPP

#include <foe/resource/create_info.h>

#include <string>

struct AnimationImportInfo {
    std::string file;
    std::string animationName;
};

struct foeArmatureCreateInfo {
    std::string fileName;
    std::string rootArmatureNode;
    uint32_t animationCount;
    AnimationImportInfo *pAnimations;
};

void cleanup_AnimationImportInfo(AnimationImportInfo *pData);

void foeDestroyArmatureCreateInfo(foeResourceCreateInfoType type, void *pCreateInfo);

#endif // ARMATURE_CREATE_INFO_HPP