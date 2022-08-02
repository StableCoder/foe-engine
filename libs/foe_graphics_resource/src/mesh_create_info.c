// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/mesh_create_info.h>

#include <foe/graphics/resource/type_defs.h>

#include <stdlib.h>

void foeCleanup_foeMeshFileCreateInfo(foeMeshFileCreateInfo *pCreateInfo) {
    if (pCreateInfo->pMesh) {
        free((char *)pCreateInfo->pMesh);
    }

    if (pCreateInfo->pFile) {
        free((char *)pCreateInfo->pFile);
    }
}