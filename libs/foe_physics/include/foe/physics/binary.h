// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_PHYSICS_BINARY_H
#define FOE_PHYSICS_BINARY_H

#include <foe/ecs/group_translator.h>
#include <foe/physics/export.h>
#include <foe/result.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foeRigidBody foeRigidBody;
typedef struct foeCollisionShapeCreateInfo foeCollisionShapeCreateInfo;

FOE_PHYSICS_EXPORT
foeResultSet binary_read_foeRigidBody(void const *pReadBuffer,
                                      uint32_t *pReadSize,
                                      foeEcsGroupTranslator groupTranslator,
                                      foeRigidBody *pData);

FOE_PHYSICS_EXPORT
foeResultSet binary_write_foeRigidBody(foeRigidBody const *pData,
                                       uint32_t *pWriteSize,
                                       void *pWriteBuffer);

FOE_PHYSICS_EXPORT
char const *binary_key_foeRigidBody();

FOE_PHYSICS_EXPORT
foeResultSet binary_read_foeCollisionShapeCreateInfo(void const *pReadBuffer,
                                                     uint32_t *pReadSize,
                                                     foeCollisionShapeCreateInfo *pData);

FOE_PHYSICS_EXPORT
foeResultSet binary_write_foeCollisionShapeCreateInfo(foeCollisionShapeCreateInfo const *pData,
                                                      uint32_t *pWriteSize,
                                                      void *pWriteBuffer);

FOE_PHYSICS_EXPORT
char const *binary_key_foeCollisionShapeCreateInfo();

#ifdef __cplusplus
}
#endif

#endif // FOE_PHYSICS_BINARY_H
