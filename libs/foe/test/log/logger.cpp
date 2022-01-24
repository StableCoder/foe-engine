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
#include <foe/log.hpp>

namespace {
struct TestSink : public foeLogSink {
  public:
    void log(foeLogCategory *, foeLogLevel level, std::string_view) { lastLogLevel = level; }
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