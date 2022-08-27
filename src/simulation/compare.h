// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BRINGUP_COMPARE_H
#define BRINGUP_COMPARE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AnimationImportInfo AnimationImportInfo;
typedef struct foeArmatureCreateInfo foeArmatureCreateInfo;

bool compare_AnimationImportInfo(AnimationImportInfo const *pData1,
                                 AnimationImportInfo const *pData2);

bool compare_foeArmatureCreateInfo(foeArmatureCreateInfo const *pData1,
                                   foeArmatureCreateInfo const *pData2);

#ifdef __cplusplus
}
#endif

#endif // BRINGUP_COMPARE_H
