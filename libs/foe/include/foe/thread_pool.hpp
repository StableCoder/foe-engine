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
    FOE_EXPORT ~foeThreadPool();

    FOE_EXPORT bool start(size_t numThreads);
    FOE_EXPORT size_t size() const noexcept;

    FOE_EXPORT void terminate();

    FOE_EXPORT void scheduleTask(std::function<void()> &&task);

    FOE_EXPORT void waitForAllTasks();

  private:
    void runThread();

    std::atomic_bool mStarted{false};
    std::vector<std::thread> mThreads;

    std::mutex mTaskSync;
    std::queue<std::function<void()>> mTasks;
    std::condition_variable mTaskAvailable;
    std::condition_variable mTaskComplete;

    std::atomic_uint mTasksProcessing{0};
    std::atomic_bool mTerminate{false};
};

#endif // FOE_THREAD_POOL_HPP