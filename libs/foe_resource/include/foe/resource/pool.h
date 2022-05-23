/*
    Copyright (C) 2022 George Cave.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

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