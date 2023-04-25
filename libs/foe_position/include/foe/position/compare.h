// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_POSITION_COMPARE_H
#define FOE_POSITION_COMPARE_H

#include <foe/position/export.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foePosition3d foePosition3d;

FOE_POSITION_EXPORT
bool compare_foePosition3d(foePosition3d const *pData1, foePosition3d const *pData2);

#ifdef __cplusplus
}
#endif

#endif // FOE_POSITION_COMPARE_H
