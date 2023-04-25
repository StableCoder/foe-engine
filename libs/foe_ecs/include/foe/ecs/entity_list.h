// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_ECS_ENTITY_LIST_H
#define FOE_ECS_ENTITY_LIST_H

#include <foe/ecs/export.h>
#include <foe/ecs/id.h>
#include <foe/handle.h>
#include <foe/result.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeEcsEntityList)

FOE_ECS_EXPORT
foeResultSet foeEcsCreateEntityList(foeEcsEntityList *pEntityList);

FOE_ECS_EXPORT
void foeEcsDestroyEntityList(foeEcsEntityList entityList);

FOE_ECS_EXPORT
foeResultSet foeEcsResetEntityList(foeEcsEntityList entityList,
                                   uint32_t listCount,
                                   uint32_t const *pEntityListCounts,
                                   foeEntityID const *const *ppEntityLists);

FOE_ECS_EXPORT
uint32_t foeEcsEntityListSize(foeEcsEntityList entityList);

FOE_ECS_EXPORT
foeEntityID const *foeEcsEntityListPtr(foeEcsEntityList entityList);

#ifdef __cplusplus
}
#endif

#endif // FOE_ECS_ENTITY_LIST_H