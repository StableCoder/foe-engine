// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RESOURCE_BINARY_H
#define FOE_GRAPHICS_RESOURCE_BINARY_H

#include <foe/ecs/group_translator.h>
#include <foe/graphics/resource/export.h>
#include <foe/result.h>

#include <stdint.h>

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
foeResultSet binary_read_foeImageCreateInfo(void const *pReadBuffer,
                                            uint32_t *pReadSize,
                                            foeImageCreateInfo *pData);

FOE_GFX_RES_EXPORT
foeResultSet binary_write_foeImageCreateInfo(foeImageCreateInfo const *pData,
                                             uint32_t *pWriteSize,
                                             void *pWriteBuffer);

FOE_GFX_RES_EXPORT
char const *binary_key_foeImageCreateInfo();

FOE_GFX_RES_EXPORT
foeResultSet binary_read_foeMeshFileCreateInfo(void const *pReadBuffer,
                                               uint32_t *pReadSize,
                                               foeMeshFileCreateInfo *pData);

FOE_GFX_RES_EXPORT
foeResultSet binary_write_foeMeshFileCreateInfo(foeMeshFileCreateInfo const *pData,
                                                uint32_t *pWriteSize,
                                                void *pWriteBuffer);

FOE_GFX_RES_EXPORT
char const *binary_key_foeMeshFileCreateInfo();

FOE_GFX_RES_EXPORT
foeResultSet binary_read_foeMeshCubeCreateInfo(void const *pReadBuffer,
                                               uint32_t *pReadSize,
                                               foeMeshCubeCreateInfo *pData);

FOE_GFX_RES_EXPORT
foeResultSet binary_write_foeMeshCubeCreateInfo(foeMeshCubeCreateInfo const *pData,
                                                uint32_t *pWriteSize,
                                                void *pWriteBuffer);

FOE_GFX_RES_EXPORT
char const *binary_key_foeMeshCubeCreateInfo();

FOE_GFX_RES_EXPORT
foeResultSet binary_read_foeMeshIcosphereCreateInfo(void const *pReadBuffer,
                                                    uint32_t *pReadSize,
                                                    foeMeshIcosphereCreateInfo *pData);

FOE_GFX_RES_EXPORT
foeResultSet binary_write_foeMeshIcosphereCreateInfo(foeMeshIcosphereCreateInfo const *pData,
                                                     uint32_t *pWriteSize,
                                                     void *pWriteBuffer);

FOE_GFX_RES_EXPORT
char const *binary_key_foeMeshIcosphereCreateInfo();

FOE_GFX_RES_EXPORT
foeResultSet binary_read_foeMaterialCreateInfo(void const *pReadBuffer,
                                               uint32_t *pReadSize,
                                               foeEcsGroupTranslator groupTranslator,
                                               foeMaterialCreateInfo *pData);

FOE_GFX_RES_EXPORT
foeResultSet binary_write_foeMaterialCreateInfo(foeMaterialCreateInfo const *pData,
                                                uint32_t *pWriteSize,
                                                void *pWriteBuffer);

FOE_GFX_RES_EXPORT
char const *binary_key_foeMaterialCreateInfo();

FOE_GFX_RES_EXPORT
foeResultSet binary_read_foeShaderCreateInfo(void const *pReadBuffer,
                                             uint32_t *pReadSize,
                                             foeShaderCreateInfo *pData);

FOE_GFX_RES_EXPORT
foeResultSet binary_write_foeShaderCreateInfo(foeShaderCreateInfo const *pData,
                                              uint32_t *pWriteSize,
                                              void *pWriteBuffer);

FOE_GFX_RES_EXPORT
char const *binary_key_foeShaderCreateInfo();

FOE_GFX_RES_EXPORT
foeResultSet binary_read_foeVertexDescriptorCreateInfo(void const *pReadBuffer,
                                                       uint32_t *pReadSize,
                                                       foeEcsGroupTranslator groupTranslator,
                                                       foeVertexDescriptorCreateInfo *pData);

FOE_GFX_RES_EXPORT
foeResultSet binary_write_foeVertexDescriptorCreateInfo(foeVertexDescriptorCreateInfo const *pData,
                                                        uint32_t *pWriteSize,
                                                        void *pWriteBuffer);

FOE_GFX_RES_EXPORT
char const *binary_key_foeVertexDescriptorCreateInfo();

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_RESOURCE_BINARY_H
