// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_SPLIT_THREAD_POOL_H
#define FOE_SPLIT_THREAD_POOL_H

#include <foe/export.h>
#include <foe/handle.h>
#include <foe/result.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Any task can be scheduled with the use of a function pointer and context data
typedef void (*PFN_foeTask)(void *);

// This can be used as a guiding pattern to be able to schedule any task with any scheduler
typedef void (*PFN_foeScheduleTask)(void *, PFN_foeTask, void *);

/** @brief A thread pool that deal with both sync and async threads
 *
 * A split thread pool creates a thread pool with both dedicated synchronous and asynchronous
 * threads based on the given count.
 *
 * However, asynchronous threads will, if they run out of asynchronous work, pick up synchronous
 * tasks while awaiting new asynchronous tasks.
 */
FOE_DEFINE_HANDLE(foeSplitThreadPool)

FOE_EXPORT
foeResultSet foeCreateThreadPool(uint32_t syncThreads,
                                 uint32_t asyncThreads,
                                 foeSplitThreadPool *pPool);

FOE_EXPORT
void foeDestroyThreadPool(foeSplitThreadPool pool);

FOE_EXPORT
uint32_t foeNumSyncThreads(foeSplitThreadPool pool);
FOE_EXPORT
uint32_t foeNumAsyncThreads(foeSplitThreadPool pool);

FOE_EXPORT
uint32_t foeNumQueuedSyncTasks(foeSplitThreadPool pool);
FOE_EXPORT
uint32_t foeNumQueuedAsyncTasks(foeSplitThreadPool pool);

FOE_EXPORT
uint32_t foeNumProcessingSyncTasks(foeSplitThreadPool pool);
FOE_EXPORT
uint32_t foeNumProcessingAsyncTasks(foeSplitThreadPool pool);

FOE_EXPORT
foeResultSet foeScheduleSyncTask(foeSplitThreadPool pool, PFN_foeTask task, void *pTaskContext);
FOE_EXPORT
foeResultSet foeScheduleAsyncTask(foeSplitThreadPool pool, PFN_foeTask task, void *pTaskContext);

FOE_EXPORT
foeResultSet foeWaitSyncThreads(foeSplitThreadPool pool);
FOE_EXPORT
foeResultSet foeWaitAsyncThreads(foeSplitThreadPool pool);
FOE_EXPORT
foeResultSet foeWaitAllThreads(foeSplitThreadPool pool);

#ifdef __cplusplus
}
#endif

#endif // FOE_SPLIT_THREAD_POOL_H