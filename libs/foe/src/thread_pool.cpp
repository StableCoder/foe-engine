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

#include <foe/thread_pool.hpp>

foeThreadPool::~foeThreadPool() { terminate(); }

bool foeThreadPool::start(size_t numThreads) {
    // Check not already running
    bool expected = false;
    if (!mStarted.compare_exchange_strong(expected, true))
        return false;

    // Make sure number of threads is not 0
    if (numThreads == 0)
        return false;

    mThreads.reserve(numThreads);
    for (size_t i = 0; i < numThreads; ++i) {
        mThreads.emplace_back(&foeThreadPool::runThread, this);
    }

    return true;
}

size_t foeThreadPool::size() const noexcept { return mThreads.size(); }

void foeThreadPool::terminate() {
    // Make sure it was started
    bool expected = true;
    if (!mStarted.compare_exchange_strong(expected, false))
        return;

    mTerminate = true;

    mTaskAvailable.notify_all();
    waitForAllTasks();

    for (auto &it : mThreads) {
        it.join();
    }

    mTerminate = false;
    mStarted = false;
}

void foeThreadPool::scheduleTask(std::function<void()> &&task) {
    mTaskSync.lock();
    mTasks.emplace(std::move(task));
    mTaskSync.unlock();

    mTaskAvailable.notify_one();
}

void foeThreadPool::waitForAllTasks() {
    std::unique_lock lock{mTaskSync};

    mTaskComplete.wait(lock, [&] { return mTasks.empty() && mTasksProcessing == 0; });
}

void foeThreadPool::runThread() {
    std::function<void()> task;
    std::unique_lock taskLock{mTaskSync, std::defer_lock};

    while (true) {
        taskLock.lock();
        if (mTasks.empty())
            mTaskAvailable.wait(taskLock, [&] { return !mTasks.empty() || mTerminate; });

        if (!mTasks.empty()) {
            // Work available
            ++mTasksProcessing;

            // Get work
            task = mTasks.front();
            mTasks.pop();

            // Release tasklist lock
            taskLock.unlock();

            // Run the task
            task();

            // Task complete, cleanup
            --mTasksProcessing;
            mTaskComplete.notify_all();
        } else if (mTerminate) {
            break;
        } else {
            taskLock.unlock();
        }
    }
}