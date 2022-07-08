// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/thread_pool.hpp>

using namespace std::chrono_literals;

foeThreadPool::~foeThreadPool() { terminate(); }

bool foeThreadPool::start(size_t numThreads) {
    // Make sure number of threads is not 0
    if (numThreads == 0)
        return false;

    // Check not already running
    bool expected = false;
    if (!mStarted.compare_exchange_strong(expected, true))
        return false;

    mThreads.reserve(numThreads);
    for (size_t i = 0; i < numThreads; ++i) {
        ++mRunners;
        mThreads.emplace_back(&foeThreadPool::runThread, this);
    }

    return true;
}

size_t foeThreadPool::size() const noexcept { return mThreads.size(); }

void foeThreadPool::terminate() {
    // Make sure it was started
    bool expected = true;
    if (!mStarted.compare_exchange_strong(expected, true))
        return;

    mTerminate = true;

    while (mRunners > 0) {
        mTaskAvailable.notify_all();
        std::this_thread::yield();
    }

    for (auto &it : mThreads) {
        it.join();
    }

    mTerminate = false;
    mStarted = false;
}

void foeThreadPool::scheduleTask(std::function<void()> &&task) {
    mTaskSync.lock();
    ++mTasksQueued;
    mTasks.emplace(std::move(task));
    mTaskSync.unlock();

    mTaskAvailable.notify_one();
}

void foeThreadPool::waitForAllTasks() {
    while (mTasksQueued > 0 || mTasksProcessing > 0) {
        std::this_thread::yield();
    }
}

void foeThreadPool::runThread() {
    std::function<void()> task;
    std::unique_lock taskLock{mTaskSync, std::defer_lock};

    while (true) {
        taskLock.lock();
        mTaskAvailable.wait_for(taskLock, 1ms, [&] { return !mTasks.empty() || mTerminate; });

        if (!mTasks.empty()) {
            // Work available
            ++mTasksProcessing;

            // Get work
            task = mTasks.front();
            mTasks.pop();
            --mTasksQueued;

            // Release tasklist lock
            taskLock.unlock();

            // Run the task
            task();

            // Task complete, cleanup
            --mTasksProcessing;
        } else if (mTerminate) {
            break;
        } else {
            taskLock.unlock();
        }
    }

    --mRunners;
}