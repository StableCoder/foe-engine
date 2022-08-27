// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BRINGUP_CLEANUP_H
#define BRINGUP_CLEANUP_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AnimationImportInfo AnimationImportInfo;
typedef struct foeArmatureCreateInfo foeArmatureCreateInfo;

void cleanup_AnimationImportInfo(AnimationImportInfo *pData);
void cleanup_foeArmatureCreateInfo(foeArmatureCreateInfo *pData);

#ifdef __cplusplus
}
#endif

#endif // BRINGUP_CLEANUP_H
