// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/chrono/program_clock.hpp>

TEST_CASE("foeProgramClock", "[foe][chrono]") {
    REQUIRE(foeProgramClock::is_steady == true);

    auto currentTime = foeProgramClock::now();

    REQUIRE(currentTime.time_since_epoch() < std::chrono::seconds(10));
}