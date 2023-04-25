// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_ECS_COMPONENT_POOL_H
#define FOE_ECS_COMPONENT_POOL_H

#include <foe/ecs/entity_list.h>
#include <foe/ecs/export.h>
#include <foe/ecs/id.h>
#include <foe/handle.h>
#include <foe/result.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeEcsComponentPool)

typedef void (*PFN_foeEcsComponentDestructor)(void *);

FOE_ECS_EXPORT
foeResultSet foeEcsCreateComponentPool(size_t initialCapacity,
                                       size_t expansionRate,
                                       size_t dataSize,
                                       PFN_foeEcsComponentDestructor dataDestructor,
                                       foeEcsComponentPool *pComponentPool);

FOE_ECS_EXPORT
void foeEcsDestroyComponentPool(foeEcsComponentPool componentPool);

FOE_ECS_EXPORT
foeResultSet foeEcsComponentPoolMaintenance(foeEcsComponentPool componentPool);

FOE_ECS_EXPORT
void foeEcsComponentPoolExpansionRate(foeEcsComponentPool componentPool, size_t expansionRate);

FOE_ECS_EXPORT
size_t foeEcsComponentPoolSize(foeEcsComponentPool componentPool);

FOE_ECS_EXPORT
foeEntityID const *foeEcsComponentPoolIdPtr(foeEcsComponentPool componentPool);

FOE_ECS_EXPORT
void *foeEcsComponentPoolDataPtr(foeEcsComponentPool componentPool);

FOE_ECS_EXPORT
size_t foeEcsComponentPoolCapacity(foeEcsComponentPool componentPool);

FOE_ECS_EXPORT
void foeEcsComponentPoolReserve(foeEcsComponentPool componentPool, size_t capacity);

FOE_ECS_EXPORT
size_t foeEcsComponentPoolInsertCapacity(foeEcsComponentPool componentPool);

FOE_ECS_EXPORT
void foeEcsComponentPoolReserveInsertCapacity(foeEcsComponentPool componentPool, size_t capacity);

FOE_ECS_EXPORT
foeResultSet foeEcsComponentPoolInsert(foeEcsComponentPool componentPool,
                                       foeEntityID entity,
                                       void *pData);

FOE_ECS_EXPORT
size_t foeEcsComponentPoolInserted(foeEcsComponentPool componentPool);

FOE_ECS_EXPORT
size_t const *foeEcsComponentPoolInsertedOffsetPtr(foeEcsComponentPool componentPool);

FOE_ECS_EXPORT
foeResultSet foeEcsComponentPoolRemove(foeEcsComponentPool componentPool, foeEntityID entity);

FOE_ECS_EXPORT
size_t foeEcsComponentPoolRemoved(foeEcsComponentPool componentPool);

FOE_ECS_EXPORT
foeEntityID const *foeEcsComponentPoolRemovedIdPtr(foeEcsComponentPool componentPool);

FOE_ECS_EXPORT
void *foeEcsComponentPoolRemovedDataPtr(foeEcsComponentPool componentPool);

FOE_ECS_EXPORT
foeResultSet foeEcsComponentPoolAddEntityList(foeEcsComponentPool componentPool,
                                              foeEcsEntityList entityList);

FOE_ECS_EXPORT
void foeEcsComponentPoolRemoveEntityList(foeEcsComponentPool componentPool,
                                         foeEcsEntityList entityList);

FOE_ECS_EXPORT
size_t foeEcsComponentPoolEntityListSize(foeEcsComponentPool componentPool);

FOE_ECS_EXPORT
foeEcsEntityList const *foeEcsComponentPoolEntityLists(foeEcsComponentPool componentPool);

#ifdef __cplusplus
}
#endif

#endif // FOE_ECS_COMPONENT_POOL_H