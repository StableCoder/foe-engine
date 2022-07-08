// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_RESOURCE_POOL_H
#define FOE_RESOURCE_POOL_H

#include <foe/ecs/id.h>
#include <foe/error_code.h>
#include <foe/handle.h>
#include <foe/resource/export.h>
#include <foe/resource/resource.h>
#include <foe/split_thread_pool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct foeResourceFns;

typedef int foeResourcePoolType;

FOE_DEFINE_HANDLE(foeResourcePool)

FOE_RES_EXPORT foeResult foeCreateResourcePool(foeResourceFns const *pResourceFns,
                                               foeResourcePool *pResourcePool);

FOE_RES_EXPORT void foeDestroyResourcePool(foeResourcePool resourcePool);

FOE_RES_EXPORT foeResource foeResourcePoolAdd(foeResourcePool resourcePool,
                                              foeResourceID resourceID,
                                              foeResourceType resourceType,
                                              size_t resourceSize);

FOE_RES_EXPORT foeResource foeResourcePoolFind(foeResourcePool resourcePool,
                                               foeResourceID resourceID);

FOE_RES_EXPORT foeResult foeResourcePoolRemove(foeResourcePool resourcePool,
                                               foeResourceID resourceID);

FOE_RES_EXPORT void foeResourcePoolAddAsyncTaskCallback(foeResourcePool resourcePool,
                                                        PFN_foeScheduleTask scheduleAsyncTask,
                                                        void *pScheduleAsyncTaskContext);

FOE_RES_EXPORT void foeResourcePoolUnloadAll(foeResourcePool resourcePool);

FOE_RES_EXPORT uint32_t foeResourcePoolUnloadType(foeResourcePool resourcePool,
                                                  foeResourceType resourceType);

#ifdef __cplusplus
}
#endif

#endif // FOE_RESOURCE_POOL_H