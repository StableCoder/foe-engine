// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/log.hpp>

namespace {
struct TestSink : public foeLogSink {
  public:
    void log(char const *, foeLogLevel level, std::string_view) { lastLogLevel = level; }
    void exception() {}

    foeLogLevel lastLogLevel;
};
} // namespace

TEST_CASE("foeLogger - Cannot register the same sink multiple times") {
    TestSink testSink;

    CHECK(foeLogger::instance()->registerSink(&testSink));
    CHECK_FALSE(foeLogger::instance()->registerSink(&testSink));

    REQUIRE(foeLogger::instance()->deregisterSink(&testSink));
}

TEST_CASE("foeLogger - Cannot deregister sink that was not registered") {
    TestSink testSink;

    CHECK_FALSE(foeLogger::instance()->deregisterSink(&testSink));
}

TEST_CASE("foeLogger - Can deregister is different orders than registered") {
    TestSink testSink, testSink2;

    CHECK(foeLogger::instance()->registerSink(&testSink));
    CHECK(foeLogger::instance()->registerSink(&testSink2));

    SECTION("Same order") {
        REQUIRE(foeLogger::instance()->deregisterSink(&testSink));
        REQUIRE(foeLogger::instance()->deregisterSink(&testSink2));
    }
    SECTION("Reverse order") {
        REQUIRE(foeLogger::instance()->deregisterSink(&testSink2));
        REQUIRE(foeLogger::instance()->deregisterSink(&testSink));
    }
}