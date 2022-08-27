// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RESOURCE_CLEANUP_H
#define FOE_GRAPHICS_RESOURCE_CLEANUP_H

#include <foe/graphics/resource/export.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foeImageCreateInfo foeImageCreateInfo;
typedef struct foeMeshFileCreateInfo foeMeshFileCreateInfo;
typedef struct foeMeshCubeCreateInfo foeMeshCubeCreateInfo;
typedef struct foeMeshIcosphereCreateInfo foeMeshIcosphereCreateInfo;
typedef struct foeMaterialCreateInfo foeMaterialCreateInfo;
typedef struct foeShaderCreateInfo foeShaderCreateInfo;
typedef struct foeVertexDescriptorCreateInfo foeVertexDescriptorCreateInfo;

FOE_GFX_RES_EXPORT void cleanup_foeImageCreateInfo(foeImageCreateInfo *pData);
FOE_GFX_RES_EXPORT void cleanup_foeMeshFileCreateInfo(foeMeshFileCreateInfo *pData);
FOE_GFX_RES_EXPORT void cleanup_foeMeshCubeCreateInfo(foeMeshCubeCreateInfo *pData);
FOE_GFX_RES_EXPORT void cleanup_foeMeshIcosphereCreateInfo(foeMeshIcosphereCreateInfo *pData);
FOE_GFX_RES_EXPORT void cleanup_foeMaterialCreateInfo(foeMaterialCreateInfo *pData);
FOE_GFX_RES_EXPORT void cleanup_foeShaderCreateInfo(foeShaderCreateInfo *pData);
FOE_GFX_RES_EXPORT void cleanup_foeVertexDescriptorCreateInfo(foeVertexDescriptorCreateInfo *pData);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_RESOURCE_CLEANUP_H
