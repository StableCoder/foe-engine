// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/log/level.hpp>

TEST_CASE("foeLogLevel - to_string checks", "[foe][log]") {
    CHECK(std::to_string(foeLogLevel::Fatal) == "Fatal");
    CHECK(std::to_string(foeLogLevel::Error) == "Error");
    CHECK(std::to_string(foeLogLevel::Warning) == "Warning");
    CHECK(std::to_string(foeLogLevel::Info) == "Info");
    CHECK(std::to_string(foeLogLevel::Verbose) == "Verbose");

    CHECK(std::to_string(static_cast<foeLogLevel>(999999)) == "Unknown Level");
}