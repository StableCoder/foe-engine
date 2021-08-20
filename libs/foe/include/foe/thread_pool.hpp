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

#ifndef FOE_THREAD_POOL_HPP
#define FOE_THREAD_POOL_HPP

#include <foe/export.h>

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class foeThreadPool {
  public:
    /// Destructs, also calls terminate() before ending.
    FOE_EXPORT ~foeThreadPool();

    /**
     * @brief Attempts to start the thread pool with the given number of threads
     * @param numThreads Number of threads to start
     * @return True if successful, false if the pool wasn't (probably was already started, or
     * numThreads was 0)
     */
    FOE_EXPORT bool start(size_t numThreads);

    /// Returns the number of threads in the pool currently (0 if not started)
    FOE_EXPORT size_t size() const noexcept;

    /**
     * @brief Terminates the pools threads after completing any waiting tasks
     * @warning Blocks until returning.
     */
    FOE_EXPORT void terminate();

    /**
     * @brief Adds a new task to be scheduled to run on this pool
     * @param task Task to be scheduled
     * @warning Tasks can be scheduled even if the pool isn't started
     */
    FOE_EXPORT void scheduleTask(std::function<void()> &&task);

    /**
     * @brief Waits for all tasks to complete then returns
     * @warning Blocks until returning.
     */
    FOE_EXPORT void waitForAllTasks();

  private:
    /**
     * @brief Function that create threads run
     *
     * A tight loop that waits for new tasks, and runs them, or end when the pool is being
     * terminated.
     */
    void runThread();

    /// Used to atomically know if the pool has been started
    std::atomic_bool mStarted{false};
    /// List of started threads
    std::vector<std::thread> mThreads;

    /// Synchronizes the task list
    std::mutex mTaskSync;
    /// Upcoming scheduled tasks
    std::queue<std::function<void()>> mTasks;
    /// Notified when a new task has been scheduled/queued
    std::condition_variable mTaskAvailable;

    // number of tasks queued awaiting processing
    std::atomic_uint mTasksQueued{0};
    /// Tracks the number of tasks currently being processed/run by the pool
    std::atomic_uint mTasksProcessing{0};
    /// Tracks if the threads have been requested to end
    std::atomic_bool mTerminate{false};
};

#endif // FOE_THREAD_POOL_HPP