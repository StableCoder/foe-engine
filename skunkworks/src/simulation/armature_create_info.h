// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef ARMATURE_CREATE_INFO_H
#define ARMATURE_CREATE_INFO_H

#include <foe/resource/create_info.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AnimationImportInfo {
    char const *pFile;
    char const *pName;
} AnimationImportInfo;

typedef struct foeArmatureCreateInfo {
    char const *pFile;
    char const *pRootArmatureNode;
    uint32_t animationCount;
    AnimationImportInfo *pAnimations;
} foeArmatureCreateInfo;

#ifdef __cplusplus
}
#endif

#endif // ARMATURE_CREATE_INFO_H