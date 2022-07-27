// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RESOURCE_MESH_CREATE_INFO_HPP
#define FOE_GRAPHICS_RESOURCE_MESH_CREATE_INFO_HPP

#include <foe/graphics/resource/export.h>
#include <foe/resource/create_info.h>

struct foeMeshFileCreateInfo {
    char const *pFile;
    char const *pMesh;
    unsigned int postProcessFlags;
};

struct foeMeshCubeCreateInfo {};

struct foeMeshIcosphereCreateInfo {
    int recursion;
};

FOE_GFX_RES_EXPORT void foeDestroyMeshCreateInfo(foeResourceCreateInfoType type, void *pCreateInfo);

#endif // FOE_GRAPHICS_RESOURCE_MESH_CREATE_INFO_HPP