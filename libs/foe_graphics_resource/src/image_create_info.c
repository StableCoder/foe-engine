// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/image_create_info.h>

#include <stdlib.h>

void foeCleanup_foeImageCreateInfo(foeImageCreateInfo *pCreateInfo) {
    if (pCreateInfo->pFile) {
        free((char *)pCreateInfo->pFile);
    }
}