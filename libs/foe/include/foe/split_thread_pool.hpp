/*
    Copyright (C) 2021 George Cave.

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

#ifndef FOE_SPLIT_THREAD_POOL_HPP
#define FOE_SPLIT_THREAD_POOL_HPP

#include <foe/export.h>
#include <foe/handle.h>

#include <cstdint>
#include <functional>
#include <system_error>

enum foeSplitThreadResult {
    FOE_THREAD_POOL_SUCCESS = 0,
    FOE_THREAD_POOL_ERROR_ZERO_SYNC_THREADS,
    FOE_THREAD_POOL_ERROR_ZERO_ASYNC_THREADS,
    FOE_THREAD_POOL_ERROR_ALLOCATION_FAILED,
    FOE_THREAD_POOL_ERROR_ALREADY_STARTED,
    FOE_THREAD_POOL_ERROR_NOT_STARTED,
};

/** @brief A thread pool that deal with both sync and async threads
 *
 * A split thread pool creates a thread pool with both dedicated synchronous and asynchronous
 * threads based on the given count.
 *
 * However, asynchronous threads will, if they run out of asynchronous work, pick up synchronous
 * tasks while awaiting new asynchronous tasks.
 */
FOE_DEFINE_HANDLE(foeSplitThreadPool)

FOE_EXPORT auto foeCreateThreadPool(uint32_t syncThreads,
                                    uint32_t asyncThreads,
                                    foeSplitThreadPool *pPool) -> std::error_code;
FOE_EXPORT void foeDestroyThreadPool(foeSplitThreadPool pool);

FOE_EXPORT auto foeStartThreadPool(foeSplitThreadPool pool) -> std::error_code;
FOE_EXPORT auto foeStopThreadPool(foeSplitThreadPool pool) -> std::error_code;

FOE_EXPORT uint32_t foeNumSyncThreads(foeSplitThreadPool pool);
FOE_EXPORT uint32_t foeNumAsyncThreads(foeSplitThreadPool pool);

FOE_EXPORT uint32_t foeNumQueuedSyncTasks(foeSplitThreadPool pool);
FOE_EXPORT uint32_t foeNumQueuedAsyncTasks(foeSplitThreadPool pool);

FOE_EXPORT uint32_t foeNumProcessingSyncTasks(foeSplitThreadPool pool);
FOE_EXPORT uint32_t foeNumProcessingAsyncTasks(foeSplitThreadPool pool);

FOE_EXPORT auto foeScheduleSyncTask(foeSplitThreadPool pool, std::function<void()> &&task)
    -> std::error_code;
FOE_EXPORT auto foeScheduleAsyncTask(foeSplitThreadPool pool, std::function<void()> &&task)
    -> std::error_code;

FOE_EXPORT auto foeWaitSyncThreads(foeSplitThreadPool pool) -> std::error_code;
FOE_EXPORT auto foeWaitAsyncThreads(foeSplitThreadPool pool) -> std::error_code;
FOE_EXPORT auto foeWaitAllThreads(foeSplitThreadPool pool) -> std::error_code;

#endif // FOE_SPLIT_THREAD_POOL_HPP