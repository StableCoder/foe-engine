// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/chrono/easy_clock.hpp>
#include <foe/thread_pool.hpp>

#include <cmath>

namespace {
auto const numThreads = 2;

void testTask() { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }
} // namespace

TEST_CASE("ThreadPool - Starting the pool") {
    foeThreadPool test;
    CHECK(test.size() == 0);

    SECTION("Starting with non-zero number succeeds") {
        REQUIRE(test.start(numThreads));
        CHECK(test.size() == numThreads);
        SECTION("Starting an already-started pool fails") { REQUIRE_FALSE(test.start(numThreads)); }
    }
    SECTION("Starting with zero number fails") {
        REQUIRE_FALSE(test.start(0));
        CHECK(test.size() == 0);
    }
}

TEST_CASE("ThreadPool - Terminating pool when not started") {
    foeThreadPool test;

    CHECK(test.size() == 0);
    test.terminate();
    CHECK(test.size() == 0);
}

TEST_CASE("ThreadPool - Waiting when no tasks queued") {
    foeThreadPool test;

    test.waitForAllTasks();
}

TEST_CASE("ThreadPool - Waiting on tasks") {
    foeThreadPool test;

    REQUIRE(test.start(numThreads));

    foeEasySteadyClock timer;

    for (int i = 0; i < 10 * numThreads; ++i) {
        test.scheduleTask(testTask);
    }

    test.waitForAllTasks();

    timer.update();

    CHECK(timer.elapsed<std::chrono::milliseconds>().count() >=
          std::chrono::milliseconds(100).count());
    // CHECK(timer.elapsed<std::chrono::milliseconds>().count() <
    //      std::chrono::milliseconds(110).count());
}

TEST_CASE("ThreadPool - Termiation") {
    foeThreadPool test;

    REQUIRE(test.start(numThreads));

    foeEasySteadyClock timer;

    for (int i = 0; i < 10 * numThreads; ++i) {
        test.scheduleTask(testTask);
    }

    test.terminate();

    timer.update();

    CHECK(timer.elapsed<std::chrono::milliseconds>().count() >=
          std::chrono::milliseconds(100).count());
    // CHECK(timer.elapsed<std::chrono::milliseconds>().count() <
    //      std::chrono::milliseconds(110).count());
}