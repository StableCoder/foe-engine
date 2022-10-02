// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/split_thread_pool.h>

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
    /// Used to atomically know if the pool has been started
    std::atomic_bool started{false};
    /// Tracks if the threads have been requested to end
    std::atomic_bool terminate{false};
    // Number of threads started
    std::atomic_uint runners{0};
    /// Threads in the pool
    std::thread *threads;

    /// Sync task data
    TaskGroupData syncTasks;
    /// Async task data
    TaskGroupData asyncTasks;
};

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

foeResultSet foeCreateThreadPool(uint32_t syncThreads,
                                 uint32_t asyncThreads,
                                 foeSplitThreadPool *pPool) {
    if (syncThreads == 0)
        return to_foeResult(FOE_ERROR_ZERO_SYNC_THREADS);
    if (asyncThreads == 0)
        return to_foeResult(FOE_ERROR_ZERO_ASYNC_THREADS);

    SplitThreadPoolImpl *pNewPool = new (std::nothrow) SplitThreadPoolImpl{
        .syncTasks =
            {
                .threadCount = syncThreads,
            },
        .asyncTasks =
            {
                .threadCount = asyncThreads,
            },
    };
    if (pNewPool == nullptr)
        return to_foeResult(FOE_ERROR_OUT_OF_MEMORY);

    foeResultSet result = to_foeResult(FOE_SUCCESS);

    pNewPool->threads =
        static_cast<std::thread *>(malloc(sizeof(std::thread) * (syncThreads + asyncThreads)));
    if (pNewPool->threads == nullptr) {
        result = to_foeResult(FOE_ERROR_OUT_OF_MEMORY);
        goto CREATE_FAILED;
    }

CREATE_FAILED:
    if (result.value != FOE_SUCCESS) {
        delete pNewPool;
    } else {
        *pPool = split_thread_pool_to_handle(pNewPool);
    }

    return result;
}

void foeDestroyThreadPool(foeSplitThreadPool pool) {
    auto *pPool = split_thread_pool_from_handle(pool);

    foeStopThreadPool(pool);
    free(pPool->threads);

    delete pPool;
}

foeResultSet foeStartThreadPool(foeSplitThreadPool pool) {
    auto *pPool = split_thread_pool_from_handle(pool);

    // Check not already running
    bool expected = false;
    if (!pPool->started.compare_exchange_strong(expected, true))
        return to_foeResult(FOE_ERROR_ALREADY_STARTED);

    // Start sync threads
    for (uint32_t i = 0; i < pPool->syncTasks.threadCount; ++i) {
        ++pPool->runners;
        new (pPool->threads + i) std::thread(syncTaskRunner, pPool);
    }

    // Start async threads
    for (uint32_t i = 0; i < pPool->asyncTasks.threadCount; ++i) {
        ++pPool->runners;
        new (pPool->threads + pPool->syncTasks.threadCount + i) std::thread(asyncTaskRunner, pPool);
    }

    return to_foeResult(FOE_SUCCESS);
}

foeResultSet foeStopThreadPool(foeSplitThreadPool pool) {
    auto *pPool = split_thread_pool_from_handle(pool);

    bool expected = true;
    if (!pPool->started.compare_exchange_strong(expected, true))
        return to_foeResult(FOE_ERROR_NOT_STARTED);

    pPool->terminate = true;

    while (pPool->runners > 0) {
        pPool->syncTasks.available.notify_all();
        pPool->asyncTasks.available.notify_all();
        std::this_thread::yield();
    }

    auto const totalThreadCount = pPool->syncTasks.threadCount + pPool->asyncTasks.threadCount;
    for (uint32_t i = 0; i < totalThreadCount; ++i) {
        pPool->threads[i].join();
    }

    pPool->terminate = false;
    pPool->started = false;

    return to_foeResult(FOE_SUCCESS);
}

uint32_t foeNumSyncThreads(foeSplitThreadPool pool) {
    auto *pPool = split_thread_pool_from_handle(pool);

    return pPool->syncTasks.threadCount;
}

uint32_t foeNumAsyncThreads(foeSplitThreadPool pool) {
    auto *pPool = split_thread_pool_from_handle(pool);

    return pPool->asyncTasks.threadCount;
}

uint32_t foeNumQueuedSyncTasks(foeSplitThreadPool pool) {
    auto *pPool = split_thread_pool_from_handle(pool);

    return pPool->syncTasks.queuedCount;
}

uint32_t foeNumQueuedAsyncTasks(foeSplitThreadPool pool) {
    auto *pPool = split_thread_pool_from_handle(pool);

    return pPool->asyncTasks.queuedCount;
}

uint32_t foeNumProcessingSyncTasks(foeSplitThreadPool pool) {
    auto *pPool = split_thread_pool_from_handle(pool);

    return pPool->syncTasks.runningCount;
}

uint32_t foeNumProcessingAsyncTasks(foeSplitThreadPool pool) {
    auto *pPool = split_thread_pool_from_handle(pool);

    return pPool->asyncTasks.runningCount;
}

foeResultSet foeScheduleSyncTask(foeSplitThreadPool pool, PFN_foeTask task, void *pTaskContext) {
    auto *pPool = split_thread_pool_from_handle(pool);

    if (!pPool->started)
        return to_foeResult(FOE_ERROR_NOT_STARTED);

    scheduleTask(pPool->syncTasks, task, pTaskContext);
    pPool->asyncTasks.available.notify_one();

    return to_foeResult(FOE_SUCCESS);
}

foeResultSet foeScheduleAsyncTask(foeSplitThreadPool pool, PFN_foeTask task, void *pTaskContext) {
    auto *pPool = split_thread_pool_from_handle(pool);

    if (!pPool->started)
        return to_foeResult(FOE_ERROR_NOT_STARTED);

    scheduleTask(pPool->asyncTasks, task, pTaskContext);

    return to_foeResult(FOE_SUCCESS);
}

foeResultSet foeWaitSyncThreads(foeSplitThreadPool pool) {
    auto *pPool = split_thread_pool_from_handle(pool);

    if (!pPool->started)
        return to_foeResult(FOE_ERROR_NOT_STARTED);

    while (pPool->syncTasks.queuedCount > 0 || pPool->syncTasks.runningCount > 0)
        std::this_thread::yield();

    return to_foeResult(FOE_SUCCESS);
}

foeResultSet foeWaitAsyncThreads(foeSplitThreadPool pool) {
    auto *pPool = split_thread_pool_from_handle(pool);

    if (!pPool->started)
        return to_foeResult(FOE_ERROR_NOT_STARTED);

    while (pPool->asyncTasks.queuedCount > 0 || pPool->asyncTasks.runningCount > 0)
        std::this_thread::yield();

    return to_foeResult(FOE_SUCCESS);
}

foeResultSet foeWaitAllThreads(foeSplitThreadPool pool) {
    auto *pPool = split_thread_pool_from_handle(pool);

    if (!pPool->started)
        return to_foeResult(FOE_ERROR_NOT_STARTED);

    while (pPool->syncTasks.queuedCount > 0 || pPool->syncTasks.runningCount > 0 ||
           pPool->asyncTasks.queuedCount > 0 || pPool->asyncTasks.runningCount > 0)
        std::this_thread::yield();

    return to_foeResult(FOE_SUCCESS);
}