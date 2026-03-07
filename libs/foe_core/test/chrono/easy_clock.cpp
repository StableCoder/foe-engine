// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/chrono/easy_clock.hpp>

#include <thread>

using namespace std::chrono_literals;

namespace {
constexpr auto cWaitTime = 50ms;
} // namespace

TEST_CASE("foeEasySystemClock", "[foe][chrono]") {
    foeEasySystemClock test;
    REQUIRE(test.currentTimePoint() == test.lastTimePoint());
    auto startPoint = test.currentTimePoint();
    auto startTime = test.currentTime<std::chrono::nanoseconds>();
    auto endPoint = startPoint + cWaitTime;

    std::chrono::nanoseconds nsAccum{0};

    while (test.currentTimePoint() < endPoint) {
        std::this_thread::sleep_for(1ms);
        test.update();
        nsAccum += test.elapsed<std::chrono::nanoseconds>();
    }

    REQUIRE(test.currentTimePoint() >= startPoint + cWaitTime);
    REQUIRE(test.currentTime<std::chrono::nanoseconds>() - startTime >= cWaitTime);
    REQUIRE(nsAccum >= cWaitTime);

    REQUIRE(test.currentTimePoint() != test.lastTimePoint());
    test.reset();
    REQUIRE(test.currentTimePoint() == test.lastTimePoint());
}

TEST_CASE("EasySteadyClock", "[foe][chrono][steady_clock]") {
    foeEasySteadyClock test;
    auto startPoint = test.currentTimePoint();
    auto startTime = test.currentTime<std::chrono::nanoseconds>();
    auto endPoint = startPoint + cWaitTime;

    std::chrono::nanoseconds nsAccum{0};

    while (test.currentTimePoint() < endPoint) {
        std::this_thread::sleep_for(1ms);
        test.update();
        nsAccum += test.elapsed<std::chrono::nanoseconds>();
    }

    REQUIRE(test.currentTimePoint() >= startPoint + cWaitTime);
    REQUIRE(test.currentTime<std::chrono::nanoseconds>() - startTime >= cWaitTime);
    REQUIRE(nsAccum >= cWaitTime);

    REQUIRE(test.currentTimePoint() != test.lastTimePoint());
    test.reset();
    REQUIRE(test.currentTimePoint() == test.lastTimePoint());
}

TEST_CASE("EasyHighResClock", "[foe][chrono][high_precision_clock]") {
    foeEasyHighResClock test;
    auto startPoint = test.currentTimePoint();
    auto startTime = test.currentTime<std::chrono::nanoseconds>();
    auto endPoint = startPoint + cWaitTime;

    std::chrono::nanoseconds nsAccum{0};

    while (test.currentTimePoint() < endPoint) {
        std::this_thread::sleep_for(1ms);
        test.update();
        nsAccum += test.elapsed<std::chrono::nanoseconds>();
    }

    REQUIRE(test.currentTimePoint() >= startPoint + cWaitTime);
    REQUIRE(test.currentTime<std::chrono::nanoseconds>() - startTime >= cWaitTime);
    REQUIRE(nsAccum >= cWaitTime);

    REQUIRE(test.currentTimePoint() != test.lastTimePoint());
    test.reset();
    REQUIRE(test.currentTimePoint() == test.lastTimePoint());
}
