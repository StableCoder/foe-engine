// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/image_create_info.h>

#include <stdlib.h>

void foeDestroyImageCreateInfo(foeResourceCreateInfoType type, void *pCreateInfo) {
    foeImageCreateInfo *pCI = (foeImageCreateInfo *)pCreateInfo;

    if (pCI->pFile)
        free(pCI->pFile);
}