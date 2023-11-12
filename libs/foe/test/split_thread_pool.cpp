// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <foe/chrono/easy_clock.hpp>
#include <foe/split_thread_pool.h>
#include <thread>

using namespace std::chrono_literals;

namespace {
auto const numThreads = 2;

void testTask(void *pContext) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    std::atomic_int *pTaskCount = (std::atomic_int *)pContext;
    ++(*pTaskCount);
}

std::atomic_uint waitingThreads{0};
std::atomic_bool pauseThreads = true;

void waitTask(void *) {
    ++waitingThreads;
    while (pauseThreads)
        std::this_thread::sleep_for(1ms);
    --waitingThreads;
}

} // namespace

TEST_CASE("SplitThreadPool - Creating the pool") {
    foeSplitThreadPool pool{FOE_NULL_HANDLE};

    SECTION("Starting with zero sync threads fails") {
        REQUIRE(foeCreateThreadPool(0, numThreads, &pool).value == FOE_ERROR_ZERO_SYNC_THREADS);
    }
    SECTION("Starting with zero async threads fails") {
        REQUIRE(foeCreateThreadPool(numThreads, 0, &pool).value == FOE_ERROR_ZERO_ASYNC_THREADS);
    }
    SECTION("Creating with non-zero number for both succeeds") {
        REQUIRE(foeCreateThreadPool(numThreads, numThreads, &pool).value == FOE_SUCCESS);
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

TEST_CASE("SplitThreadPool - Waiting when no tasks queued") {
    foeSplitThreadPool pool{FOE_NULL_HANDLE};
    REQUIRE(foeCreateThreadPool(numThreads, numThreads, &pool).value == FOE_SUCCESS);

    SECTION("Waiting on pool with no tasks has no issues") {
        REQUIRE(foeWaitSyncThreads(pool).value == FOE_SUCCESS);
        REQUIRE(foeWaitAsyncThreads(pool).value == FOE_SUCCESS);
        REQUIRE(foeWaitAllThreads(pool).value == FOE_SUCCESS);
    }

    foeDestroyThreadPool(pool);
}

TEST_CASE("SplitThreadPool - Checking task queries") {
    foeSplitThreadPool pool{FOE_NULL_HANDLE};
    REQUIRE(foeCreateThreadPool(numThreads, numThreads, &pool).value == FOE_SUCCESS);

    pauseThreads = true;

    SECTION("Sync tasks (checking that async also take sync jobs)") {
        CHECK(foeScheduleSyncTask(pool, waitTask, nullptr).value == FOE_SUCCESS);
        while (waitingThreads < 1)
            ;
        CHECK(foeNumQueuedSyncTasks(pool) == 0);
        CHECK(foeNumQueuedAsyncTasks(pool) == 0);
        CHECK((foeNumProcessingSyncTasks(pool) + foeNumProcessingAsyncTasks(pool)) == 1);

        for (int i = 0; i < numThreads * 2; ++i)
            CHECK(foeScheduleSyncTask(pool, waitTask, nullptr).value == FOE_SUCCESS);
        while (waitingThreads < numThreads * 2)
            ;
        CHECK(foeNumQueuedSyncTasks(pool) == 1);
        CHECK(foeNumQueuedAsyncTasks(pool) == 0);
        CHECK((foeNumProcessingSyncTasks(pool) + foeNumProcessingAsyncTasks(pool)) ==
              numThreads * 2);
    }

    SECTION("Async tasks") {
        CHECK(foeScheduleAsyncTask(pool, waitTask, nullptr).value == FOE_SUCCESS);
        while (waitingThreads < 1)
            ;
        CHECK(foeNumQueuedSyncTasks(pool) == 0);
        CHECK(foeNumProcessingSyncTasks(pool) == 0);
        CHECK(foeNumQueuedAsyncTasks(pool) == 0);
        CHECK(foeNumProcessingAsyncTasks(pool) == 1);

        for (int i = 0; i < numThreads; ++i)
            CHECK(foeScheduleAsyncTask(pool, waitTask, nullptr).value == FOE_SUCCESS);
        while (waitingThreads < numThreads)
            ;
        CHECK(foeNumQueuedSyncTasks(pool) == 0);
        CHECK(foeNumProcessingSyncTasks(pool) == 0);
        CHECK(foeNumQueuedAsyncTasks(pool) == 1);
        CHECK(foeNumProcessingAsyncTasks(pool) == numThreads);
    }

    SECTION("Both task types threads") {
        for (int i = 0; i < numThreads + 1; ++i)
            CHECK(foeScheduleAsyncTask(pool, waitTask, nullptr).value == FOE_SUCCESS);
        while (waitingThreads < numThreads)
            ;
        for (int i = 0; i < numThreads + 1; ++i)
            CHECK(foeScheduleSyncTask(pool, waitTask, nullptr).value == FOE_SUCCESS);
        while (waitingThreads < numThreads * 2)
            ;
        CHECK(foeNumQueuedSyncTasks(pool) == 1);
        CHECK(foeNumProcessingSyncTasks(pool) == numThreads);
        CHECK(foeNumQueuedAsyncTasks(pool) == 1);
        CHECK(foeNumProcessingAsyncTasks(pool) == numThreads);
    }

    pauseThreads = false;

    foeDestroyThreadPool(pool);
}

TEST_CASE("SplitThreadPool - Waiting on sync tasks") {
    foeSplitThreadPool pool{FOE_NULL_HANDLE};
    REQUIRE(foeCreateThreadPool(numThreads, numThreads, &pool).value == FOE_SUCCESS);

    std::atomic_int taskCount = 0;
    foeEasySteadyClock timer;

    for (int i = 0; i < 10 * numThreads; ++i) {
        REQUIRE(foeScheduleSyncTask(pool, testTask, &taskCount).value == FOE_SUCCESS);
    }

    REQUIRE(foeWaitSyncThreads(pool).value == FOE_SUCCESS);

    timer.update();

    CHECK(taskCount == 10 * numThreads);
    // !! half time since async threads accelerate sync work !!
    CHECK(timer.elapsed<std::chrono::milliseconds>().count() >=
          std::chrono::milliseconds(50).count());

    foeDestroyThreadPool(pool);
}

TEST_CASE("SplitThreadPool - Waiting on async tasks") {
    foeSplitThreadPool pool{FOE_NULL_HANDLE};
    REQUIRE(foeCreateThreadPool(numThreads, numThreads, &pool).value == FOE_SUCCESS);

    std::atomic_int taskCount = 0;
    foeEasySteadyClock timer;

    for (int i = 0; i < 10 * numThreads; ++i) {
        REQUIRE(foeScheduleAsyncTask(pool, testTask, &taskCount).value == FOE_SUCCESS);
    }

    REQUIRE(foeWaitAsyncThreads(pool).value == FOE_SUCCESS);

    timer.update();

    CHECK(taskCount == 10 * numThreads);
    CHECK(timer.elapsed<std::chrono::milliseconds>().count() >=
          std::chrono::milliseconds(100).count());

    foeDestroyThreadPool(pool);
}

TEST_CASE("SplitThreadPool - Waiting on all tasks") {
    foeSplitThreadPool pool{FOE_NULL_HANDLE};
    REQUIRE(foeCreateThreadPool(numThreads, numThreads, &pool).value == FOE_SUCCESS);

    std::atomic_int taskCount = 0;
    foeEasySteadyClock timer;

    for (int i = 0; i < 10 * numThreads; ++i) {
        REQUIRE(foeScheduleSyncTask(pool, testTask, &taskCount).value == FOE_SUCCESS);
        REQUIRE(foeScheduleAsyncTask(pool, testTask, &taskCount).value == FOE_SUCCESS);
    }

    REQUIRE(foeWaitAllThreads(pool).value == FOE_SUCCESS);

    timer.update();

    CHECK(taskCount == 10 * numThreads * 2);
    CHECK(timer.elapsed<std::chrono::milliseconds>().count() >=
          std::chrono::milliseconds(100).count());

    foeDestroyThreadPool(pool);
}

TEST_CASE("SplitThreadPool - Destroying pool awaits completion of all tasks") {
    foeSplitThreadPool pool{FOE_NULL_HANDLE};
    REQUIRE(foeCreateThreadPool(numThreads, numThreads, &pool).value == FOE_SUCCESS);

    std::atomic_int taskCount = 0;
    foeEasySteadyClock timer;

    for (int i = 0; i < 10 * numThreads; ++i) {
        REQUIRE(foeScheduleSyncTask(pool, testTask, &taskCount).value == FOE_SUCCESS);
        REQUIRE(foeScheduleAsyncTask(pool, testTask, &taskCount).value == FOE_SUCCESS);
    }

    foeDestroyThreadPool(pool);

    timer.update();

    CHECK(taskCount == 10 * numThreads * 2);
    CHECK(timer.elapsed<std::chrono::milliseconds>().count() >=
          std::chrono::milliseconds(100).count());
}