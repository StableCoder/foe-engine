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

#ifndef FOE_SPLIT_THREAD_POOL_H
#define FOE_SPLIT_THREAD_POOL_H

#include <foe/error_code.h>
#include <foe/export.h>
#include <foe/handle.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum foeSplitThreadResult {
    FOE_THREAD_POOL_SUCCESS = 0,
    FOE_THREAD_POOL_ERROR_ZERO_SYNC_THREADS,
    FOE_THREAD_POOL_ERROR_ZERO_ASYNC_THREADS,
    FOE_THREAD_POOL_ERROR_ALLOCATION_FAILED,
    FOE_THREAD_POOL_ERROR_ALREADY_STARTED,
    FOE_THREAD_POOL_ERROR_NOT_STARTED,
};

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

FOE_EXPORT foeErrorCode foeCreateThreadPool(uint32_t syncThreads,
                                            uint32_t asyncThreads,
                                            foeSplitThreadPool *pPool);
FOE_EXPORT void foeDestroyThreadPool(foeSplitThreadPool pool);

FOE_EXPORT foeErrorCode foeStartThreadPool(foeSplitThreadPool pool);
FOE_EXPORT foeErrorCode foeStopThreadPool(foeSplitThreadPool pool);

FOE_EXPORT uint32_t foeNumSyncThreads(foeSplitThreadPool pool);
FOE_EXPORT uint32_t foeNumAsyncThreads(foeSplitThreadPool pool);

FOE_EXPORT uint32_t foeNumQueuedSyncTasks(foeSplitThreadPool pool);
FOE_EXPORT uint32_t foeNumQueuedAsyncTasks(foeSplitThreadPool pool);

FOE_EXPORT uint32_t foeNumProcessingSyncTasks(foeSplitThreadPool pool);
FOE_EXPORT uint32_t foeNumProcessingAsyncTasks(foeSplitThreadPool pool);

FOE_EXPORT foeErrorCode foeScheduleSyncTask(foeSplitThreadPool pool,
                                            PFN_foeTask task,
                                            void *pTaskContext);
FOE_EXPORT foeErrorCode foeScheduleAsyncTask(foeSplitThreadPool pool,
                                             PFN_foeTask task,
                                             void *pTaskContext);

FOE_EXPORT foeErrorCode foeWaitSyncThreads(foeSplitThreadPool pool);
FOE_EXPORT foeErrorCode foeWaitAsyncThreads(foeSplitThreadPool pool);
FOE_EXPORT foeErrorCode foeWaitAllThreads(foeSplitThreadPool pool);

#ifdef __cplusplus
}
#endif

#endif // FOE_SPLIT_THREAD_POOL_H