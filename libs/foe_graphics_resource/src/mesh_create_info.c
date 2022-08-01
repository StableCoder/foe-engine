// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/mesh_create_info.h>

#include <foe/graphics/resource/type_defs.h>

#include <stdlib.h>

void foeDestroyMeshCreateInfo(foeResourceCreateInfoType type, void *pCreateInfo) {
    if (type == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_FILE_CREATE_INFO) {
        foeMeshFileCreateInfo *pCI = (foeMeshFileCreateInfo *)pCreateInfo;

        if (pCI->pMesh)
            free(pCI->pMesh);
        if (pCI->pFile)
            free(pCI->pFile);
    } else if (type == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_CUBE_CREATE_INFO) {
        // Do nothing
    } else if (type == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_FILE_CREATE_INFO) {
        // Do nothing
    }
}