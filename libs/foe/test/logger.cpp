// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/log.h>

namespace {

foeLogLevel lastLogLevel;

void log(void *, char const *, foeLogLevel level, char const *) { lastLogLevel = level; }
void exception(void *) {}

} // namespace

TEST_CASE("foeLogLevel - to_string checks", "[foe][log]") {
    CHECK(std::string_view{foeLogLevel_to_string(FOE_LOG_LEVEL_FATAL)} == "Fatal");
    CHECK(std::string_view{foeLogLevel_to_string(FOE_LOG_LEVEL_ERROR)} == "Error");
    CHECK(std::string_view{foeLogLevel_to_string(FOE_LOG_LEVEL_WARNING)} == "Warning");
    CHECK(std::string_view{foeLogLevel_to_string(FOE_LOG_LEVEL_INFO)} == "Info");
    CHECK(std::string_view{foeLogLevel_to_string(FOE_LOG_LEVEL_VERBOSE)} == "Verbose");

    CHECK(std::string_view{foeLogLevel_to_string(static_cast<foeLogLevel>(999999))} == "Unknown");
}

TEST_CASE("foeLogger - Cannot register the same sink multiple times") {
    CHECK(foeLogRegisterSink(nullptr, log, exception));
    CHECK_FALSE(foeLogRegisterSink(nullptr, log, exception));

    REQUIRE(foeLogDeregisterSink(nullptr, log, exception));
}

TEST_CASE("foeLogger - Cannot deregister sink that was not registered") {
    CHECK_FALSE(foeLogDeregisterSink(nullptr, log, exception));
}

TEST_CASE("foeLogger - Can deregister is different orders than registered") {
    CHECK(foeLogRegisterSink(nullptr, log, exception));
    CHECK(foeLogRegisterSink(&lastLogLevel, log, exception));

    SECTION("Same order") {
        REQUIRE(foeLogDeregisterSink(nullptr, log, exception));
        REQUIRE(foeLogDeregisterSink(&lastLogLevel, log, exception));
    }
    SECTION("Reverse order") {
        REQUIRE(foeLogDeregisterSink(&lastLogLevel, log, exception));
        REQUIRE(foeLogDeregisterSink(nullptr, log, exception));
    }
}