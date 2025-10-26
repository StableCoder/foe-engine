// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BRINGUP_BINARY_H
#define BRINGUP_BINARY_H

#include <foe/ecs/group_translator.h>
#include <foe/result.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AnimationImportInfo AnimationImportInfo;
typedef struct foeArmatureCreateInfo foeArmatureCreateInfo;
typedef struct foeArmatureState foeArmatureState;
typedef struct foeRenderState foeRenderState;

foeResultSet binary_read_AnimationImportInfo(void const *pReadBuffer,
                                             uint32_t *pReadSize,
                                             AnimationImportInfo *pData);

foeResultSet binary_write_AnimationImportInfo(AnimationImportInfo const *pData,
                                              uint32_t *pWriteSize,
                                              void *pWriteBuffer);

char const *binary_key_AnimationImportInfo();

foeResultSet binary_read_foeArmatureCreateInfo(void const *pReadBuffer,
                                               uint32_t *pReadSize,
                                               foeArmatureCreateInfo *pData);

foeResultSet binary_write_foeArmatureCreateInfo(foeArmatureCreateInfo const *pData,
                                                uint32_t *pWriteSize,
                                                void *pWriteBuffer);

char const *binary_key_foeArmatureCreateInfo();

foeResultSet binary_read_foeArmatureState(void const *pReadBuffer,
                                          uint32_t *pReadSize,
                                          foeEcsGroupTranslator groupTranslator,
                                          foeArmatureState *pData);

foeResultSet binary_write_foeArmatureState(foeArmatureState const *pData,
                                           uint32_t *pWriteSize,
                                           void *pWriteBuffer);

char const *binary_key_foeArmatureState();

foeResultSet binary_read_foeRenderState(void const *pReadBuffer,
                                        uint32_t *pReadSize,
                                        foeEcsGroupTranslator groupTranslator,
                                        foeRenderState *pData);

foeResultSet binary_write_foeRenderState(foeRenderState const *pData,
                                         uint32_t *pWriteSize,
                                         void *pWriteBuffer);

char const *binary_key_foeRenderState();

#ifdef __cplusplus
}
#endif

#endif // BRINGUP_BINARY_H
