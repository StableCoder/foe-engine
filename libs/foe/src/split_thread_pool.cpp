// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/split_thread_pool.h>

#ifdef _WIN32
#include <windows.h>

#include <timeapi.h>
#endif

#include "result.h"

#include <atomic>
#include <condition_variable>
#include <queue>
#include <thread>

using namespace std::chrono_literals;

namespace {

struct Task {
    PFN_foeTask task;
    void *pTaskContext;
};

struct TaskGroupData {
    /// Number of threads in the task group
    uint32_t threadCount;

    /// Synchronizes the task list
    std::mutex sync;
    /// Upcoming scheduled tasks
    std::queue<Task> tasks;
    /// Notified when a new task has been scheduled/queued
    std::condition_variable available;
    /// Number of tasks queued
    std::atomic_uint queuedCount;
    /// Number of tasks running/processing
    std::atomic_uint runningCount;
};

void scheduleTask(TaskGroupData &taskGroup, PFN_foeTask task, void *pTaskContext) {
    taskGroup.sync.lock();
    ++taskGroup.queuedCount;
    taskGroup.tasks.emplace(Task{
        .task = task,
        .pTaskContext = pTaskContext,
    });
    taskGroup.sync.unlock();

    taskGroup.available.notify_one();
}

struct SplitThreadPoolImpl {
    /// Tracks if the threads have been requested to end
    std::atomic_bool terminate;
    // Number of threads started
    std::atomic_uint runners;

    /// Sync task data
    TaskGroupData syncTasks;
    /// Async task data
    TaskGroupData asyncTasks;
};

inline std::thread *getThreadArray(SplitThreadPoolImpl *pThreadPool) {
    return (std::thread *)(((uint8_t *)pThreadPool) + sizeof(SplitThreadPoolImpl));
}

FOE_DEFINE_HANDLE_CASTS(split_thread_pool, SplitThreadPoolImpl, foeSplitThreadPool)

void syncTaskRunner(SplitThreadPoolImpl *pPool) {
    Task task;
    std::unique_lock syncLock{pPool->syncTasks.sync, std::defer_lock};

    while (true) {
        syncLock.lock();
        pPool->syncTasks.available.wait_for(
            syncLock, 1ms, [&] { return !pPool->syncTasks.tasks.empty() || pPool->terminate; });

        if (!pPool->syncTasks.tasks.empty()) {
            // Work available
            ++pPool->syncTasks.runningCount;

            // Get work
            task = pPool->syncTasks.tasks.front();
            pPool->syncTasks.tasks.pop();
            --pPool->syncTasks.queuedCount;

            // Release tasklist lock
            syncLock.unlock();

            // Run the task
            task.task(task.pTaskContext);

            // Task complete, cleanup
            --pPool->syncTasks.runningCount;
        } else if (pPool->terminate) {
            break;
        } else {
            syncLock.unlock();
        }
    }

    --pPool->runners;
}

void asyncTaskRunner(SplitThreadPoolImpl *pPool) {
    Task task;
    std::unique_lock asyncLock{pPool->asyncTasks.sync, std::defer_lock};

    while (true) {
        asyncLock.lock();
        pPool->asyncTasks.available.wait_for(asyncLock, 1ms, [&] {
            return !pPool->asyncTasks.tasks.empty() || pPool->syncTasks.queuedCount > 0 ||
                   pPool->terminate;
        });

        if (!pPool->asyncTasks.tasks.empty()) {
            // Work available
            ++pPool->asyncTasks.runningCount;

            // Get work
            task = std::move(pPool->asyncTasks.tasks.front());
            pPool->asyncTasks.tasks.pop();
            --pPool->asyncTasks.queuedCount;

            // Release tasklist lock
            asyncLock.unlock();

            // Run the task
            task.task(task.pTaskContext);

            // Task complete, cleanup
            --pPool->asyncTasks.runningCount;
        } else if (pPool->syncTasks.queuedCount > 0 && pPool->syncTasks.sync.try_lock()) {
            // Might have synchronous work available, check quickly
            asyncLock.unlock();

            bool gotWork{false};
            if (!pPool->syncTasks.tasks.empty()) {
                gotWork = true;
                ++pPool->syncTasks.runningCount;

                task = std::move(pPool->syncTasks.tasks.front());
                pPool->syncTasks.tasks.pop();
                --pPool->syncTasks.queuedCount;
            }
            pPool->syncTasks.sync.unlock();

            if (gotWork) {
                task.task(task.pTaskContext);
                --pPool->syncTasks.runningCount;
            }
        } else if (pPool->terminate) {
            break;
        } else {
            asyncLock.unlock();
        }
    }

    --pPool->runners;
}

} // namespace

extern "C" foeResultSet foeCreateThreadPool(uint32_t syncThreads,
                                            uint32_t asyncThreads,
                                            foeSplitThreadPool *pPool) {
    if (syncThreads == 0)
        return to_foeResult(FOE_ERROR_ZERO_SYNC_THREADS);
    if (asyncThreads == 0)
        return to_foeResult(FOE_ERROR_ZERO_ASYNC_THREADS);

    // Allocation is the implementation + (total num threads * std::thread) so that it is one
    // contiguous allocation
    SplitThreadPoolImpl *pNewPool = (SplitThreadPoolImpl *)malloc(
        sizeof(SplitThreadPoolImpl) + (syncThreads + asyncThreads) * sizeof(std::thread));
    if (pNewPool == nullptr)
        return to_foeResult(FOE_ERROR_OUT_OF_MEMORY);

#ifdef _WIN32
    // On Windows, set the kernel rate to 1000Hz
    timeBeginPeriod(1);
#endif

    new (pNewPool) SplitThreadPoolImpl{
        .terminate = false,
        .runners = 0,
        .syncTasks =
            {
                .threadCount = syncThreads,
            },
        .asyncTasks =
            {
                .threadCount = asyncThreads,
            },
    };

    std::thread *pThreads = getThreadArray(pNewPool);
    // Start sync threads
    for (uint32_t i = 0; i < pNewPool->syncTasks.threadCount; ++i) {
        ++pNewPool->runners;
        new (pThreads + i) std::thread(syncTaskRunner, pNewPool);
    }

    // Start async threads
    for (uint32_t i = 0; i < pNewPool->asyncTasks.threadCount; ++i) {
        ++pNewPool->runners;
        new (pThreads + pNewPool->syncTasks.threadCount + i) std::thread(asyncTaskRunner, pNewPool);
    }

    *pPool = split_thread_pool_to_handle(pNewPool);

    return to_foeResult(FOE_SUCCESS);
}

extern "C" void foeDestroyThreadPool(foeSplitThreadPool pool) {
    auto *pPool = split_thread_pool_from_handle(pool);

    // Terminate running threads
    pPool->terminate = true;

    while (pPool->runners > 0) {
        pPool->syncTasks.available.notify_all();
        pPool->asyncTasks.available.notify_all();
        std::this_thread::yield();
    }

    std::thread *pThreads = getThreadArray(pPool);
    auto const totalThreadCount = pPool->syncTasks.threadCount + pPool->asyncTasks.threadCount;
    for (uint32_t i = 0; i < totalThreadCount; ++i) {
        pThreads[i].join();
    }

#ifdef _WIN32
    // On Windows, unset the kernel rate to 1000Hz
    timeEndPeriod(1);
#endif

    // Free used memory
    pPool->~SplitThreadPoolImpl();
    free(pPool);
}

extern "C" uint32_t foeNumSyncThreads(foeSplitThreadPool pool) {
    auto *pPool = split_thread_pool_from_handle(pool);

    return pPool->syncTasks.threadCount;
}

extern "C" uint32_t foeNumAsyncThreads(foeSplitThreadPool pool) {
    auto *pPool = split_thread_pool_from_handle(pool);

    return pPool->asyncTasks.threadCount;
}

extern "C" uint32_t foeNumQueuedSyncTasks(foeSplitThreadPool pool) {
    auto *pPool = split_thread_pool_from_handle(pool);

    return pPool->syncTasks.queuedCount;
}

extern "C" uint32_t foeNumQueuedAsyncTasks(foeSplitThreadPool pool) {
    auto *pPool = split_thread_pool_from_handle(pool);

    return pPool->asyncTasks.queuedCount;
}

extern "C" uint32_t foeNumProcessingSyncTasks(foeSplitThreadPool pool) {
    auto *pPool = split_thread_pool_from_handle(pool);

    return pPool->syncTasks.runningCount;
}

extern "C" uint32_t foeNumProcessingAsyncTasks(foeSplitThreadPool pool) {
    auto *pPool = split_thread_pool_from_handle(pool);

    return pPool->asyncTasks.runningCount;
}

extern "C" foeResultSet foeScheduleSyncTask(foeSplitThreadPool pool,
                                            PFN_foeTask task,
                                            void *pTaskContext) {
    auto *pPool = split_thread_pool_from_handle(pool);

    scheduleTask(pPool->syncTasks, task, pTaskContext);
    pPool->asyncTasks.available.notify_one();

    return to_foeResult(FOE_SUCCESS);
}

extern "C" foeResultSet foeScheduleAsyncTask(foeSplitThreadPool pool,
                                             PFN_foeTask task,
                                             void *pTaskContext) {
    auto *pPool = split_thread_pool_from_handle(pool);

    scheduleTask(pPool->asyncTasks, task, pTaskContext);

    return to_foeResult(FOE_SUCCESS);
}

extern "C" foeResultSet foeWaitSyncThreads(foeSplitThreadPool pool) {
    auto *pPool = split_thread_pool_from_handle(pool);

    while (pPool->syncTasks.queuedCount > 0 || pPool->syncTasks.runningCount > 0)
        std::this_thread::yield();

    return to_foeResult(FOE_SUCCESS);
}

extern "C" foeResultSet foeWaitAsyncThreads(foeSplitThreadPool pool) {
    auto *pPool = split_thread_pool_from_handle(pool);

    while (pPool->asyncTasks.queuedCount > 0 || pPool->asyncTasks.runningCount > 0)
        std::this_thread::yield();

    return to_foeResult(FOE_SUCCESS);
}

extern "C" foeResultSet foeWaitAllThreads(foeSplitThreadPool pool) {
    auto *pPool = split_thread_pool_from_handle(pool);

    while (pPool->syncTasks.queuedCount > 0 || pPool->syncTasks.runningCount > 0 ||
           pPool->asyncTasks.queuedCount > 0 || pPool->asyncTasks.runningCount > 0)
        std::this_thread::yield();

    return to_foeResult(FOE_SUCCESS);
}