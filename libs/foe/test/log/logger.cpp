// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/log.hpp>

namespace {

foeLogLevel lastLogLevel;

void log(void *, char const *, foeLogLevel level, char const *) { lastLogLevel = level; }
void exception(void *) {}

} // namespace

TEST_CASE("foeLogger - Cannot register the same sink multiple times") {
    CHECK(foeLogger::instance()->registerSink(nullptr, log, exception));
    CHECK_FALSE(foeLogger::instance()->registerSink(nullptr, log, exception));

    REQUIRE(foeLogger::instance()->deregisterSink(nullptr, log, exception));
}

TEST_CASE("foeLogger - Cannot deregister sink that was not registered") {
    CHECK_FALSE(foeLogger::instance()->deregisterSink(nullptr, log, exception));
}

TEST_CASE("foeLogger - Can deregister is different orders than registered") {
    CHECK(foeLogger::instance()->registerSink(nullptr, log, exception));
    CHECK(foeLogger::instance()->registerSink(&lastLogLevel, log, exception));

    SECTION("Same order") {
        REQUIRE(foeLogger::instance()->deregisterSink(nullptr, log, exception));
        REQUIRE(foeLogger::instance()->deregisterSink(&lastLogLevel, log, exception));
    }
    SECTION("Reverse order") {
        REQUIRE(foeLogger::instance()->deregisterSink(&lastLogLevel, log, exception));
        REQUIRE(foeLogger::instance()->deregisterSink(nullptr, log, exception));
    }
}