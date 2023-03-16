// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/chrono/dilated_clock.hpp>

using namespace std::chrono_literals;

TEST_CASE("foeDilatedClock - Test default constructed state.", "[foe][chrono]") {
    foeDilatedClock clock;

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 0);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>().count() == 0);
    REQUIRE(clock.dilation() == 1.0f);
}

TEST_CASE("foeDilatedClock - Test constructed items with custom parameters", "[foe][chrono]") {
    SECTION("Only a given start time.") {
        foeDilatedClock clock(std::chrono::seconds(6005));

        REQUIRE(clock.time<std::chrono::milliseconds>() == std::chrono::seconds(6005));
        REQUIRE(clock.elapsed<std::chrono::milliseconds>() == 0ms);
        REQUIRE(clock.dilation() == 1.0f);

        foeDilatedClock clock2(std::chrono::hours(150654));

        REQUIRE(clock2.time<std::chrono::milliseconds>() == std::chrono::hours(150654));
        REQUIRE(clock2.elapsed<std::chrono::milliseconds>() == 0ms);
        REQUIRE(clock2.dilation() == 1.0f);
    }

    SECTION("Both a given start time and time dilation", "[foe][chrono]") {
        foeDilatedClock clock(0s, 1.0f);

        REQUIRE(clock.time<std::chrono::milliseconds>() == 0s);
        REQUIRE(clock.elapsed<std::chrono::milliseconds>() == 0ms);
        REQUIRE(clock.dilation() == 1.0f);

        foeDilatedClock clock2(std::chrono::hours(150654), 0.5f);

        REQUIRE(clock2.time<std::chrono::milliseconds>() == std::chrono::hours(150654));
        REQUIRE(clock2.elapsed<std::chrono::milliseconds>() == 0ms);
        REQUIRE(clock2.dilation() == 0.5f);

        foeDilatedClock clock3(std::chrono::seconds(6005), 3.105f);

        REQUIRE(clock3.time<std::chrono::milliseconds>() == std::chrono::seconds(6005));
        REQUIRE(clock3.elapsed<std::chrono::milliseconds>() == 0ms);
        REQUIRE(clock3.dilation() == 3.105f);

        foeDilatedClock clock4(std::chrono::minutes(150654), 10.0f);

        REQUIRE(clock4.time<std::chrono::milliseconds>() == std::chrono::minutes(150654));
        REQUIRE(clock4.elapsed<std::chrono::milliseconds>() == 0ms);
        REQUIRE(clock4.dilation() == 10.0f);
    }
}

TEST_CASE("foeDilatedClock - Testing setting time via function.", "[foe][chrono]") {
    foeDilatedClock clock;

    clock.time(std::chrono::milliseconds(5430));

    REQUIRE(clock.time<std::chrono::milliseconds>() == std::chrono::milliseconds(5430));
    REQUIRE(clock.elapsed<std::chrono::milliseconds>() == 0ms);
    REQUIRE(clock.dilation() == 1.0f);

    clock.time(std::chrono::seconds(5430));

    REQUIRE(clock.time<std::chrono::milliseconds>() == std::chrono::seconds(5430));
    REQUIRE(clock.elapsed<std::chrono::milliseconds>() == 0ms);
    REQUIRE(clock.dilation() == 1.0f);

    clock.time(std::chrono::minutes(5430));

    REQUIRE(clock.time<std::chrono::milliseconds>() == std::chrono::minutes(5430));
    REQUIRE(clock.elapsed<std::chrono::milliseconds>() == 0ms);
    REQUIRE(clock.dilation() == 1.0f);

    clock.time(std::chrono::hours(5430));

    REQUIRE(clock.time<std::chrono::milliseconds>() == std::chrono::hours(5430));
    REQUIRE(clock.elapsed<std::chrono::milliseconds>() == 0ms);
    REQUIRE(clock.dilation() == 1.0f);
}

TEST_CASE("foeDilatedClock - Testing setting dilation via function.", "[foe][chrono]") {
    foeDilatedClock clock;

    clock.dilation(2.0f);

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 0);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>().count() == 0);
    REQUIRE(clock.dilation() == 2.0f);

    clock.dilation(110.05f);

    REQUIRE(clock.time<std::chrono::milliseconds>() == 0s);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>() == 0ms);
    REQUIRE(clock.dilation() == 110.05f);

    clock.dilation(0.0f);

    REQUIRE(clock.time<std::chrono::milliseconds>() == std::chrono::minutes(0));
    REQUIRE(clock.elapsed<std::chrono::milliseconds>() == 0ms);
    REQUIRE(clock.dilation() == 0.0f);

    clock.dilation(1.0f);

    REQUIRE(clock.time<std::chrono::milliseconds>() == std::chrono::hours(0));
    REQUIRE(clock.elapsed<std::chrono::milliseconds>() == 0ms);
    REQUIRE(clock.dilation() == 1.0f);
}

TEST_CASE("foeDilatedClock - Testing the clock updates with dilation of 1.0f", "[foe][chrono]") {
    foeDilatedClock clock;

    clock.update(10ms, 10ms);

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 10);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>().count() == 10);
    REQUIRE(clock.dilation() == 1.0f);

    clock.update(std::chrono::milliseconds(20), 10ms);

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 20);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>().count() == 10);
    REQUIRE(clock.dilation() == 1.0f);

    clock.update(std::chrono::milliseconds(30), 10ms);

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 30);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>().count() == 10);
    REQUIRE(clock.dilation() == 1.0f);

    clock.update(std::chrono::milliseconds(40), 10ms);

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 40);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>().count() == 10);
    REQUIRE(clock.dilation() == 1.0f);
}

TEST_CASE("foeDilatedClock - Testing the clock updates with dilation of 2.0f", "[foe][chrono]") {
    foeDilatedClock clock;
    clock.dilation(2.0f);
    REQUIRE(clock.dilation() == 2.0f);

    clock.update(100ms, 100ms);

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 200);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>().count() == 200);

    clock.update(200ms, 100ms);

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 400);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>().count() == 200);

    clock.update(300ms, 100ms);

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 600);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>().count() == 200);

    clock.update(400ms, 100ms);

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 800);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>().count() == 200);
}

TEST_CASE("foeDilatedClock - Testing the clock updates with dilation of 1.5f", "[foe][chrono]") {
    foeDilatedClock clock;
    clock.dilation(1.5f);
    REQUIRE(clock.dilation() == 1.5f);

    clock.update(100ms, 100ms);

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 150);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>().count() == 150);

    clock.update(200ms, 100ms);

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 300);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>().count() == 150);

    clock.update(300ms, 100ms);

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 450);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>().count() == 150);

    clock.update(400ms, 100ms);

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 600);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>().count() == 150);
}

TEST_CASE("foeDilatedClock - Testing the clock updates with dilation of 0.0001f", "[foe][chrono]") {
    foeDilatedClock clock;
    clock.dilation(0.001f);
    REQUIRE(clock.dilation() == 0.001f);

    for (int i = 0; i < 1001; ++i) {
        clock.update(std::chrono::milliseconds((i + 1) * 10), 10ms);
    }

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 10);
}

TEST_CASE("foeDilatedClock - Testing the clock updates with dilation of 0.0001f and time steps "
          "that would error out to zero",
          "[foe][chrono]") {
    foeDilatedClock clock;
    clock.dilation(0.000001f);
    REQUIRE(clock.dilation() == 0.000001f);

    auto externalClock = 0ns;
    while (externalClock < 51ms) {
        clock.update(externalClock + 999990ns, 999990ns);
        externalClock += 999990ns;
    }

    REQUIRE(clock.time<std::chrono::nanoseconds>().count() == 50);
}

TEST_CASE("foeDilatedClock - Testing the clock updates with dilation of 0.f", "[foe][chrono]") {
    foeDilatedClock clock;
    clock.dilation(0.f);
    REQUIRE(clock.dilation() == 0.f);

    for (int i = 0; i < 1001; ++i) {
        clock.update(std::chrono::milliseconds((i + 1) * 10), 10ms);
    }

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 0);
}