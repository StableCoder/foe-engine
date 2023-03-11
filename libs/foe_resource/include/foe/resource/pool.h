// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_RESOURCE_POOL_H
#define FOE_RESOURCE_POOL_H

#include <foe/ecs/id.h>
#include <foe/handle.h>
#include <foe/resource/export.h>
#include <foe/resource/resource.h>
#include <foe/result.h>
#include <foe/split_thread_pool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct foeResourceFns;

typedef int foeResourcePoolType;

FOE_DEFINE_HANDLE(foeResourcePool)

FOE_RES_EXPORT foeResultSet foeCreateResourcePool(foeResourceFns const *pResourceFns,
                                                  foeResourcePool *pResourcePool);

FOE_RES_EXPORT void foeDestroyResourcePool(foeResourcePool resourcePool);

// Returned resources have reference count pre-incremented.
FOE_RES_EXPORT foeResource foeResourcePoolAdd(foeResourcePool resourcePool,
                                              foeResourceID resourceID);

// Returned resources have reference count pre-incremented.
FOE_RES_EXPORT foeResource foeResourcePoolLoadedReplace(
    foeResourcePool resourcePool,
    foeResourceID resourceID,
    foeResourceType resourceType,
    size_t resourceSize,
    void *pLoadDataContext,
    PFN_foeResourceDataModify loadDataFn,
    void *pUnloadDataContext,
    void (*pUnloadDataFn)(void *, foeResource, uint32_t, PFN_foeResourceUnloadCall, bool));

// Returned resources have reference count pre-incremented.
FOE_RES_EXPORT foeResource foeResourcePoolFind(foeResourcePool resourcePool,
                                               foeResourceID resourceID);

FOE_RES_EXPORT foeResultSet foeResourcePoolRemove(foeResourcePool resourcePool,
                                                  foeResourceID resourceID);

FOE_RES_EXPORT void foeResourcePoolSetAsyncTaskCallback(foeResourcePool resourcePool,
                                                        void *pScheduleAsyncTaskContext,
                                                        PFN_foeScheduleTask scheduleAsyncTask);

// Unloads called 'immediately'
FOE_RES_EXPORT void foeResourcePoolUnloadAll(foeResourcePool resourcePool);

// Unloads called 'immediately'
FOE_RES_EXPORT uint32_t foeResourcePoolUnloadType(foeResourcePool resourcePool,
                                                  foeResourceType resourceType);

#ifdef __cplusplus
}
#endif

#endif // FOE_RESOURCE_POOL_H