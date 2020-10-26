/*
    Copyright (C) 2020 George Cave.

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
#include <foe/chrono/dilated_long_clock.hpp>

using namespace std::chrono_literals;

TEST_CASE("foeDilatedLongClock - Default constructed state.", "[foe][chrono]") {
    foeDilatedLongClock clock{std::chrono::seconds{0}};

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 0);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>().count() == 0);
    REQUIRE(clock.dilation() == 1.0f);
}

TEST_CASE("foeDilatedLongClock - Constructor given a start time.", "[foe][chrono]") {
    foeDilatedLongClock clock(std::chrono::seconds(0), std::chrono::seconds(6005));

    REQUIRE(clock.time<std::chrono::milliseconds>() == std::chrono::seconds(6005));
    REQUIRE(clock.elapsed<std::chrono::milliseconds>() == std::chrono::milliseconds(0));
    REQUIRE(clock.dilation() == 1.0f);

    foeDilatedLongClock clock2(std::chrono::seconds(0), std::chrono::hours(150654));

    REQUIRE(clock2.time<std::chrono::milliseconds>() == std::chrono::hours(150654));
    REQUIRE(clock2.elapsed<std::chrono::milliseconds>() == std::chrono::milliseconds(0));
    REQUIRE(clock2.dilation() == 1.0f);
}

TEST_CASE("foeDilatedLongClock - Constructor given start time and time dilation", "[foe][chrono]") {
    foeDilatedLongClock clock(std::chrono::seconds(0), std::chrono::seconds(0), 1.0f);

    REQUIRE(clock.time<std::chrono::milliseconds>() == std::chrono::seconds(0));
    REQUIRE(clock.elapsed<std::chrono::milliseconds>() == std::chrono::milliseconds(0));
    REQUIRE(clock.dilation() == 1.0f);

    foeDilatedLongClock clock2(std::chrono::seconds(0), std::chrono::hours(150654), 0.5f);

    REQUIRE(clock2.time<std::chrono::milliseconds>() == std::chrono::hours(150654));
    REQUIRE(clock2.elapsed<std::chrono::milliseconds>() == std::chrono::milliseconds(0));
    REQUIRE(clock2.dilation() == 0.5f);

    foeDilatedLongClock clock3(std::chrono::seconds(0), std::chrono::seconds(6005), 3.105f);

    REQUIRE(clock3.time<std::chrono::milliseconds>() == std::chrono::seconds(6005));
    REQUIRE(clock3.elapsed<std::chrono::milliseconds>() == std::chrono::milliseconds(0));
    REQUIRE(clock3.dilation() == 3.105f);

    foeDilatedLongClock clock4(std::chrono::seconds(0), std::chrono::minutes(150654), 10.0f);

    REQUIRE(clock4.time<std::chrono::milliseconds>() == std::chrono::minutes(150654));
    REQUIRE(clock4.elapsed<std::chrono::milliseconds>() == std::chrono::milliseconds(0));
    REQUIRE(clock4.dilation() == 10.0f);
}

TEST_CASE("foeDilatedLongClock - Setting time via function.", "[foe][chrono]") {
    foeDilatedLongClock clock{std::chrono::seconds{0}};

    clock.time(std::chrono::milliseconds(5430));

    REQUIRE(clock.time<std::chrono::milliseconds>() == std::chrono::milliseconds(5430));
    REQUIRE(clock.elapsed<std::chrono::milliseconds>() == std::chrono::milliseconds(0));
    REQUIRE(clock.dilation() == 1.0f);

    clock.time(std::chrono::seconds(5430));

    REQUIRE(clock.time<std::chrono::milliseconds>() == std::chrono::seconds(5430));
    REQUIRE(clock.elapsed<std::chrono::milliseconds>() == std::chrono::milliseconds(0));
    REQUIRE(clock.dilation() == 1.0f);

    clock.time(std::chrono::minutes(5430));

    REQUIRE(clock.time<std::chrono::milliseconds>() == std::chrono::minutes(5430));
    REQUIRE(clock.elapsed<std::chrono::milliseconds>() == std::chrono::milliseconds(0));
    REQUIRE(clock.dilation() == 1.0f);

    clock.time(std::chrono::hours(5430));

    REQUIRE(clock.time<std::chrono::milliseconds>() == std::chrono::hours(5430));
    REQUIRE(clock.elapsed<std::chrono::milliseconds>() == std::chrono::milliseconds(0));
    REQUIRE(clock.dilation() == 1.0f);
}

TEST_CASE("foeDilatedLongClock - Setting dilation via function.", "[foe][chrono]") {
    foeDilatedLongClock clock{std::chrono::seconds{0}};

    clock.dilation(2.0f);

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 0);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>().count() == 0);
    REQUIRE(clock.dilation() == 2.0f);

    clock.dilation(110.05f);

    REQUIRE(clock.time<std::chrono::milliseconds>() == std::chrono::seconds(0));
    REQUIRE(clock.elapsed<std::chrono::milliseconds>() == std::chrono::milliseconds(0));
    REQUIRE(clock.dilation() == 110.05f);

    clock.dilation(0.0f);

    REQUIRE(clock.time<std::chrono::milliseconds>() == std::chrono::minutes(0));
    REQUIRE(clock.elapsed<std::chrono::milliseconds>() == std::chrono::milliseconds(0));
    REQUIRE(clock.dilation() == 0.0f);

    clock.dilation(1.0f);

    REQUIRE(clock.time<std::chrono::milliseconds>() == std::chrono::hours(0));
    REQUIRE(clock.elapsed<std::chrono::milliseconds>() == std::chrono::milliseconds(0));
    REQUIRE(clock.dilation() == 1.0f);
}

TEST_CASE("foeDilatedLongClock - Clock updates with dilation of 1.0f", "[foe][chrono]") {
    foeDilatedLongClock clock{std::chrono::seconds{0}};

    clock.update(std::chrono::milliseconds(10));

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 10);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>().count() == 10);
    REQUIRE(clock.dilation() == 1.0f);

    clock.update(std::chrono::milliseconds(20));

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 20);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>().count() == 10);
    REQUIRE(clock.dilation() == 1.0f);

    clock.update(std::chrono::milliseconds(30));

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 30);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>().count() == 10);
    REQUIRE(clock.dilation() == 1.0f);

    clock.update(std::chrono::milliseconds(40));

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 40);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>().count() == 10);
    REQUIRE(clock.dilation() == 1.0f);
}

TEST_CASE("foeDilatedLongClock - Clock updates with dilation of 2.0f", "[foe][chrono]") {
    foeDilatedLongClock clock{std::chrono::seconds{0}};
    clock.dilation(2.0f);
    REQUIRE(clock.dilation() == 2.0f);

    clock.update(std::chrono::milliseconds(100));

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 200);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>().count() == 200);

    clock.update(std::chrono::milliseconds(200));

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 400);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>().count() == 200);

    clock.update(std::chrono::milliseconds(300));

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 600);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>().count() == 200);

    clock.update(std::chrono::milliseconds(400));

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 800);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>().count() == 200);
}

TEST_CASE("foeDilatedLongClock - Clock updates with dilation of 1.5f", "[foe][chrono]") {
    foeDilatedLongClock clock{std::chrono::seconds{0}};
    clock.dilation(1.5f);
    REQUIRE(clock.dilation() == 1.5f);

    clock.update(std::chrono::milliseconds(100));

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 150);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>().count() == 150);

    clock.update(std::chrono::milliseconds(200));

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 300);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>().count() == 150);

    clock.update(std::chrono::milliseconds(300));

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 450);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>().count() == 150);

    clock.update(std::chrono::milliseconds(400));

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 600);
    REQUIRE(clock.elapsed<std::chrono::milliseconds>().count() == 150);
}

TEST_CASE("foeDilatedLongClock - Clock updates with dilation of 0.0001f", "[foe][chrono]") {
    foeDilatedLongClock clock{std::chrono::seconds{0}};
    clock.dilation(0.001f);
    REQUIRE(clock.dilation() == 0.001f);

    for (int i = 0; i < 1001; ++i) {
        clock.update(std::chrono::milliseconds((i + 1) * 10));
    }

    REQUIRE(clock.time<std::chrono::milliseconds>().count() == 10);
}

TEST_CASE("foeDilatedLongClock - Changing external time doesn't affect internal time",
          "[foe][chrono]") {
    foeDilatedLongClock clock{std::chrono::seconds{0}};

    clock.update(100ms);

    REQUIRE(clock.elapsed<std::chrono::milliseconds>() == 100ms);
    REQUIRE(clock.time<std::chrono::milliseconds>() == 100ms);
    REQUIRE(clock.externalTime() == 100ms);

    clock.externalTime(5000ms);
    clock.update(5100ms);

    REQUIRE(clock.elapsed<std::chrono::milliseconds>() == 100ms);
    REQUIRE(clock.time<std::chrono::milliseconds>() == 200ms);
    REQUIRE(clock.externalTime() == 5100ms);
}