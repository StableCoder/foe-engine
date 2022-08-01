// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/shader_create_info.h>

#include <stdlib.h>

void foeDestroyShaderCreateInfo(foeResourceCreateInfoType type, void *pCreateInfo) {
    foeShaderCreateInfo *pCI = (foeShaderCreateInfo *)pCreateInfo;

    foeGfxVkDestroyShaderCreateInfo(&pCI->gfxCreateInfo);

    if (pCI->pFile)
        free(pCI->pFile);
}