// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RESOURCE_COMPARE_H
#define FOE_GRAPHICS_RESOURCE_COMPARE_H

#include <foe/graphics/resource/export.h>

#include <stdbool.h>

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

FOE_GFX_RES_EXPORT
bool compare_foeImageCreateInfo(foeImageCreateInfo const *pData1, foeImageCreateInfo const *pData2);

FOE_GFX_RES_EXPORT
bool compare_foeMeshFileCreateInfo(foeMeshFileCreateInfo const *pData1,
                                   foeMeshFileCreateInfo const *pData2);

FOE_GFX_RES_EXPORT
bool compare_foeMeshCubeCreateInfo(foeMeshCubeCreateInfo const *pData1,
                                   foeMeshCubeCreateInfo const *pData2);

FOE_GFX_RES_EXPORT
bool compare_foeMeshIcosphereCreateInfo(foeMeshIcosphereCreateInfo const *pData1,
                                        foeMeshIcosphereCreateInfo const *pData2);

FOE_GFX_RES_EXPORT
bool compare_foeMaterialCreateInfo(foeMaterialCreateInfo const *pData1,
                                   foeMaterialCreateInfo const *pData2);

FOE_GFX_RES_EXPORT
bool compare_foeShaderCreateInfo(foeShaderCreateInfo const *pData1,
                                 foeShaderCreateInfo const *pData2);

FOE_GFX_RES_EXPORT
bool compare_foeVertexDescriptorCreateInfo(foeVertexDescriptorCreateInfo const *pData1,
                                           foeVertexDescriptorCreateInfo const *pData2);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_RESOURCE_COMPARE_H
