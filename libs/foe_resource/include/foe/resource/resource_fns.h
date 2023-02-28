// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_RESOURCE_RESOURCE_FNS_H
#define FOE_RESOURCE_RESOURCE_FNS_H

#include <foe/ecs/id.h>
#include <foe/resource/create_info.h>
#include <foe/resource/resource.h>
#include <foe/split_thread_pool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef foeResourceCreateInfo (*PFN_foeGetResourceCreateInfoData)(void *, foeResourceID);
typedef void (*PFN_foeLoadResourceData)(void *,
                                        foeResource,
                                        foeResourceCreateInfo,
                                        PFN_foeResourcePostLoad *);

/**
 * Set of functions common to all foeResource types for importing definitions, loading data and
 * making the importation and loading run asynchronously.
 */
struct foeResourceFns {
    void *pImportContext;
    PFN_foeGetResourceCreateInfoData pImportFn;
    void *pLoadContext;
    PFN_foeLoadResourceData pLoadFn;
    PFN_foeScheduleTask scheduleAsyncTask;
    void *pScheduleAsyncTaskContext;
};

#ifdef __cplusplus
}
#endif

#endif // FOE_RESOURCE_RESOURCE_FNS_H