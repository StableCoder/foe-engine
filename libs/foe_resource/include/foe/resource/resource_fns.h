// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_RESOURCE_RESOURCE_FNS_H
#define FOE_RESOURCE_RESOURCE_FNS_H

#include <foe/ecs/id.h>
#include <foe/resource/resource.h>
#include <foe/split_thread_pool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*PFN_foeLoadResourceData)(void *, foeResource, PFN_foeResourcePostLoad);

/**
 * Set of functions common to all foeResource types for loading data and to run loading calls
 * asynchronously.
 */
struct foeResourceFns {
    void *pLoadContext;
    PFN_foeLoadResourceData pLoadFn;
    PFN_foeScheduleTask scheduleAsyncTask;
    void *pScheduleAsyncTaskContext;
};

#ifdef __cplusplus
}
#endif

#endif // FOE_RESOURCE_RESOURCE_FNS_H