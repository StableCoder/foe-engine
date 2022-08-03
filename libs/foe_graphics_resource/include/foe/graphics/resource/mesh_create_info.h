// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RESOURCE_MESH_CREATE_INFO_H
#define FOE_GRAPHICS_RESOURCE_MESH_CREATE_INFO_H

#include <foe/graphics/resource/export.h>
#include <foe/resource/create_info.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foeMeshFileCreateInfo {
    char const *pFile;
    char const *pMesh;
    unsigned int postProcessFlags;
} foeMeshFileCreateInfo;

typedef struct foeMeshCubeCreateInfo {
    void *padding;
} foeMeshCubeCreateInfo;

typedef struct foeMeshIcosphereCreateInfo {
    int recursion;
} foeMeshIcosphereCreateInfo;

FOE_GFX_RES_EXPORT void foeCleanup_foeMeshFileCreateInfo(foeMeshFileCreateInfo *pCreateInfo);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_RESOURCE_MESH_CREATE_INFO_H