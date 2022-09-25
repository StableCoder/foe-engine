// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_ECS_BINARY_H
#define FOE_ECS_BINARY_H

#include <foe/ecs/export.h>
#include <foe/ecs/group_translator.h>
#include <foe/ecs/id.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_ECS_EXPORT foeResultSet binary_read_foeResourceID(void const *pReadBuffer,
                                                      uint32_t *pReadSize,
                                                      foeEcsGroupTranslator groupTranslator,
                                                      foeResourceID *pResourceID);

FOE_ECS_EXPORT foeResultSet binary_write_foeResourceID(foeResourceID resourceID,
                                                       uint32_t *pWriteSize,
                                                       void *pWriteBuffer);

FOE_ECS_EXPORT foeResultSet binary_read_foeEntityID(void const *pReadBuffer,
                                                    uint32_t *pReadSize,
                                                    foeEcsGroupTranslator groupTranslator,
                                                    foeEntityID *pEntityID);

FOE_ECS_EXPORT foeResultSet binary_write_foeEntityID(foeEntityID resourceID,
                                                     uint32_t *pWriteSize,
                                                     void *pWriteBuffer);

#ifdef __cplusplus
}
#endif

#endif // FOE_ECS_BINARY_H