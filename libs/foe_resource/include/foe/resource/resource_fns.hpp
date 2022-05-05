/*
    Copyright (C) 2021-2022 George Cave.

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

#ifndef FOE_RESOURCE_RESOURCE_FNS_HPP
#define FOE_RESOURCE_RESOURCE_FNS_HPP

#include <foe/ecs/id.h>
#include <foe/resource/create_info.h>
#include <foe/resource/resource.h>
#include <foe/split_thread_pool.h>

/**
 * Set of functions common to all foeResource types for importing definitions, loading data and
 * making the importation and loading run asynchronously.
 */
struct foeResourceFns {
    void *pImportContext;
    foeResourceCreateInfo (*pImportFn)(void *, foeResourceID);
    void *pLoadContext;
    void (*pLoadFn)(void *, foeResource, PFN_foeResourcePostLoad *);
    PFN_foeScheduleTask scheduleAsyncTask;
    void *pScheduleAsyncTaskContext;
};

#endif // FOE_RESOURCE_RESOURCE_FNS_HPP