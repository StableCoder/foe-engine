// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/shader_create_info.h>

#include <stdlib.h>

void foeCleanup_foeShaderCreateInfo(foeShaderCreateInfo *pCreateInfo) {
    foeCleanup_foeGfxVkShaderCreateInfo(&pCreateInfo->gfxCreateInfo);

    if (pCreateInfo->pFile) {
        free((char *)pCreateInfo->pFile);
    }
}