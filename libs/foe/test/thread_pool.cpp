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
#include <foe/thread_pool.hpp>

#include <cmath>

auto const numThreads = 2;

void testTask() { std::this_thread::sleep_for(std::chrono::milliseconds(50)); }

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

    for (int i = 0; i < 20 * numThreads; ++i) {
        test.scheduleTask(testTask);
    }

    test.waitForAllTasks();

    timer.update();

    CHECK(timer.elapsed<std::chrono::milliseconds>().count() >=
          std::chrono::milliseconds(1000).count());
    CHECK(timer.elapsed<std::chrono::milliseconds>().count() <
          std::chrono::milliseconds(1300).count());
}

TEST_CASE("ThreadPool - Termiation") {
    foeThreadPool test;

    REQUIRE(test.start(numThreads));

    foeEasySteadyClock timer;

    for (int i = 0; i < 20 * numThreads; ++i) {
        test.scheduleTask(testTask);
    }

    test.terminate();

    timer.update();

    CHECK(timer.elapsed<std::chrono::milliseconds>().count() >=
          std::chrono::milliseconds(1000).count());
    CHECK(timer.elapsed<std::chrono::milliseconds>().count() <
          std::chrono::milliseconds(1300).count());
}