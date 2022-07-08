// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef ARMATURE_CREATE_INFO_HPP
#define ARMATURE_CREATE_INFO_HPP

#include <foe/resource/create_info.h>

#include <string>
#include <vector>

struct AnimationImportInfo {
    std::string file;
    std::vector<std::string> animationNames;
};

struct foeArmatureCreateInfo {
    std::string fileName;
    std::string rootArmatureNode;
    std::vector<AnimationImportInfo> animations;
};

void foeDestroyArmatureCreateInfo(foeResourceCreateInfoType type, void *pCreateInfo);

#endif // ARMATURE_CREATE_INFO_HPP