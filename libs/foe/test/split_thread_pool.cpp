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

#include <catch.hpp>
#include <foe/chrono/easy_clock.hpp>
#include <foe/split_thread_pool.hpp>

#include <thread>

namespace {
auto const numThreads = 2;

void testTask() { std::this_thread::sleep_for(std::chrono::milliseconds(50)); }
} // namespace

TEST_CASE("SplitThreadPool - Creating the pool") {
    foeSplitThreadPool pool{FOE_NULL_HANDLE};

    SECTION("Starting with zero sync threads fails") {
        REQUIRE(foeCreateThreadPool(0, numThreads, &pool).value() ==
                FOE_THREAD_POOL_ERROR_ZERO_SYNC_THREADS);
    }
    SECTION("Starting with zero async threads fails") {
        REQUIRE(foeCreateThreadPool(numThreads, 0, &pool).value() ==
                FOE_THREAD_POOL_ERROR_ZERO_ASYNC_THREADS);
    }
    SECTION("Creating with non-zero number for both succeeds") {
        REQUIRE_FALSE(foeCreateThreadPool(numThreads, numThreads, &pool));
        CHECK(foeNumSyncThreads(pool) == numThreads);
        CHECK(foeNumAsyncThreads(pool) == numThreads);

        CHECK(foeNumQueuedSyncTasks(pool) == 0);
        CHECK(foeNumQueuedAsyncTasks(pool) == 0);
        CHECK(foeNumProcessingSyncTasks(pool) == 0);
        CHECK(foeNumProcessingAsyncTasks(pool) == 0);
    }

    if (pool != FOE_NULL_HANDLE) {
        foeDestroyThreadPool(pool);
    }
}

TEST_CASE("SplitThreadPool - Starting/stopping the pool") {
    foeSplitThreadPool pool{FOE_NULL_HANDLE};
    REQUIRE_FALSE(foeCreateThreadPool(numThreads, numThreads, &pool));

    SECTION("Starting the pool when not started succeeds") {
        REQUIRE_FALSE(foeStartThreadPool(pool));

        SECTION("Attempting to start an already started pool fails") {
            REQUIRE(foeStartThreadPool(pool).value() == FOE_THREAD_POOL_ERROR_ALREADY_STARTED);
        }

        REQUIRE_FALSE(foeStopThreadPool(pool));

        SECTION("Restarting a stopped thread pool succeeds") {
            REQUIRE_FALSE(foeStartThreadPool(pool));
        }
    }

    SECTION("Stopping a non-started pool fails") {
        REQUIRE(foeStopThreadPool(pool).value() == FOE_THREAD_POOL_ERROR_NOT_STARTED);
    }

    foeDestroyThreadPool(pool);
}

TEST_CASE("SplitThreadPool - Waiting when no tasks queued") {
    foeSplitThreadPool pool{FOE_NULL_HANDLE};
    REQUIRE_FALSE(foeCreateThreadPool(numThreads, numThreads, &pool));

    SECTION("Waiting on started pool has no issues") {
        REQUIRE_FALSE(foeStartThreadPool(pool));

        REQUIRE_FALSE(foeWaitSyncThreads(pool));
        REQUIRE_FALSE(foeWaitAsyncThreads(pool));
        REQUIRE_FALSE(foeWaitAllThreads(pool));
    }
    SECTION("Waiting on non-started pool fails") {
        SECTION("sync wait") {
            REQUIRE(foeWaitSyncThreads(pool).value() == FOE_THREAD_POOL_ERROR_NOT_STARTED);
        }
        SECTION("async wait") {
            REQUIRE(foeWaitAsyncThreads(pool).value() == FOE_THREAD_POOL_ERROR_NOT_STARTED);
        }
        SECTION("all wait") {
            REQUIRE(foeWaitAllThreads(pool).value() == FOE_THREAD_POOL_ERROR_NOT_STARTED);
        }
    }

    foeDestroyThreadPool(pool);
}

TEST_CASE("SplitThreadPool - Scheduling tasks when not started fails") {
    foeSplitThreadPool pool{FOE_NULL_HANDLE};
    REQUIRE_FALSE(foeCreateThreadPool(numThreads, numThreads, &pool));

    REQUIRE(foeScheduleSyncTask(pool, testTask).value() == FOE_THREAD_POOL_ERROR_NOT_STARTED);
    REQUIRE(foeScheduleAsyncTask(pool, testTask).value() == FOE_THREAD_POOL_ERROR_NOT_STARTED);

    foeDestroyThreadPool(pool);
}

TEST_CASE("SplitThreadPool - Waiting on sync tasks") {
    foeSplitThreadPool pool{FOE_NULL_HANDLE};
    REQUIRE_FALSE(foeCreateThreadPool(numThreads, numThreads, &pool));
    REQUIRE_FALSE(foeStartThreadPool(pool));

    foeEasySteadyClock timer;

    for (int i = 0; i < 20 * numThreads; ++i) {
        // !! Double tasks since async threads accelerate sync work !!
        REQUIRE_FALSE(foeScheduleSyncTask(pool, testTask));
    }

    CHECK(foeNumQueuedSyncTasks(pool) > 0);
    CHECK(foeNumProcessingSyncTasks(pool) > 0);

    CHECK(foeNumQueuedAsyncTasks(pool) == 0);
    CHECK(foeNumProcessingAsyncTasks(pool) == 0);

    REQUIRE_FALSE(foeWaitSyncThreads(pool));

    timer.update();

    CHECK(timer.elapsed<std::chrono::milliseconds>().count() >=
          std::chrono::milliseconds(500).count());
    // CHECK(timer.elapsed<std::chrono::milliseconds>().count() <
    //      std::chrono::milliseconds(650).count());

    foeDestroyThreadPool(pool);
}

TEST_CASE("SplitThreadPool - Waiting on async tasks") {
    foeSplitThreadPool pool{FOE_NULL_HANDLE};
    REQUIRE_FALSE(foeCreateThreadPool(numThreads, numThreads, &pool));
    REQUIRE_FALSE(foeStartThreadPool(pool));

    foeEasySteadyClock timer;

    for (int i = 0; i < 10 * numThreads; ++i) {
        REQUIRE_FALSE(foeScheduleAsyncTask(pool, testTask));
    }

    CHECK(foeNumQueuedSyncTasks(pool) == 0);
    CHECK(foeNumProcessingSyncTasks(pool) == 0);

    CHECK(foeNumQueuedAsyncTasks(pool) > 0);
    CHECK(foeNumProcessingAsyncTasks(pool) > 0);

    REQUIRE_FALSE(foeWaitAsyncThreads(pool));

    timer.update();

    CHECK(timer.elapsed<std::chrono::milliseconds>().count() >=
          std::chrono::milliseconds(500).count());
    // CHECK(timer.elapsed<std::chrono::milliseconds>().count() <
    //      std::chrono::milliseconds(650).count());

    foeDestroyThreadPool(pool);
}

TEST_CASE("SplitThreadPool - Waiting on all tasks") {
    foeSplitThreadPool pool{FOE_NULL_HANDLE};
    REQUIRE_FALSE(foeCreateThreadPool(numThreads, numThreads, &pool));
    REQUIRE_FALSE(foeStartThreadPool(pool));

    foeEasySteadyClock timer;

    for (int i = 0; i < 10 * numThreads; ++i) {
        REQUIRE_FALSE(foeScheduleSyncTask(pool, testTask));
        REQUIRE_FALSE(foeScheduleAsyncTask(pool, testTask));
    }

    CHECK(foeNumQueuedSyncTasks(pool) > 0);
    CHECK(foeNumProcessingSyncTasks(pool) > 0);

    CHECK(foeNumQueuedAsyncTasks(pool) > 0);
    CHECK(foeNumProcessingAsyncTasks(pool) > 0);

    REQUIRE_FALSE(foeWaitAllThreads(pool));

    timer.update();

    CHECK(timer.elapsed<std::chrono::milliseconds>().count() >=
          std::chrono::milliseconds(500).count());
    // CHECK(timer.elapsed<std::chrono::milliseconds>().count() <
    //      std::chrono::milliseconds(650).count());

    foeDestroyThreadPool(pool);
}

TEST_CASE("SplitThreadPool - Destroying pool awaits completion of all tasks") {
    foeSplitThreadPool pool{FOE_NULL_HANDLE};
    REQUIRE_FALSE(foeCreateThreadPool(numThreads, numThreads, &pool));
    REQUIRE_FALSE(foeStartThreadPool(pool));

    foeEasySteadyClock timer;

    for (int i = 0; i < 10 * numThreads; ++i) {
        REQUIRE_FALSE(foeScheduleSyncTask(pool, testTask));
        REQUIRE_FALSE(foeScheduleAsyncTask(pool, testTask));
    }

    CHECK(foeNumQueuedSyncTasks(pool) > 0);
    CHECK(foeNumProcessingSyncTasks(pool) > 0);

    CHECK(foeNumQueuedAsyncTasks(pool) > 0);
    CHECK(foeNumProcessingAsyncTasks(pool) > 0);

    foeDestroyThreadPool(pool);

    timer.update();

    CHECK(timer.elapsed<std::chrono::milliseconds>().count() >=
          std::chrono::milliseconds(500).count());
    // CHECK(timer.elapsed<std::chrono::milliseconds>().count() <
    //      std::chrono::milliseconds(650).count());
}