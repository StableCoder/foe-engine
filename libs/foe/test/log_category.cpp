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

FOE_DECLARE_LOG_CATEGORY(TestCategory, All, All)
FOE_DEFINE_LOG_CATEGORY(TestCategory)

TEST_CASE("foeLogCategory Functionality", "[foe][foeLogCategory]") {
    auto *testCategory = TestCategory::instance();

    REQUIRE(testCategory->name() == "TestCategory");
    REQUIRE(testCategory->maxLevel() == foeLogLevel::All);

    testCategory->maxLevel(foeLogLevel::Error);
    REQUIRE(testCategory->maxLevel() == foeLogLevel::Error);
}

/// Testing compile levels
namespace {
struct TestSink : public foeLogSink {
  public:
    void log(foeLogCategory *, foeLogLevel level, std::string_view) { lastLogLevel = level; }
    void exception() {}

    foeLogLevel lastLogLevel;
};
} // namespace

FOE_DECLARE_LOG_CATEGORY(FatalCategory, All, Fatal)
FOE_DEFINE_LOG_CATEGORY(FatalCategory)

TEST_CASE("foeLogCategory - Fatal compilation level") {
    TestSink testSink;
    foeLogger::instance()->registerSink(&testSink);

    FOE_LOG(FatalCategory, Fatal, "Fatal");
    FOE_LOG(FatalCategory, Error, "Error");
    FOE_LOG(FatalCategory, Warning, "Warning");
    FOE_LOG(FatalCategory, Info, "Info");
    FOE_LOG(FatalCategory, Verbose, "Verbose");

    foeLogger::instance()->deregisterSink(&testSink);

    REQUIRE(testSink.lastLogLevel == foeLogLevel::Fatal);
}

FOE_DECLARE_LOG_CATEGORY(WarningCategory, All, Warning)
FOE_DEFINE_LOG_CATEGORY(WarningCategory)

TEST_CASE("foeLogCategory - Warning compilation level") {
    TestSink testSink;
    foeLogger::instance()->registerSink(&testSink);

    FOE_LOG(WarningCategory, Fatal, "Fatal");
    FOE_LOG(WarningCategory, Error, "Error");
    FOE_LOG(WarningCategory, Warning, "Warning");
    FOE_LOG(WarningCategory, Info, "Info");
    FOE_LOG(WarningCategory, Verbose, "Verbose");

    foeLogger::instance()->deregisterSink(&testSink);

    REQUIRE(testSink.lastLogLevel == foeLogLevel::Warning);
}

FOE_DECLARE_LOG_CATEGORY(VerboseCategory, All, Verbose)
FOE_DEFINE_LOG_CATEGORY(VerboseCategory)

TEST_CASE("foeLogCategory - Verbose compilation level") {
    TestSink testSink;
    foeLogger::instance()->registerSink(&testSink);

    FOE_LOG(VerboseCategory, Fatal, "Fatal");
    FOE_LOG(VerboseCategory, Error, "Error");
    FOE_LOG(VerboseCategory, Warning, "Warning");
    FOE_LOG(VerboseCategory, Info, "Info");
    FOE_LOG(VerboseCategory, Verbose, "Verbose");

    foeLogger::instance()->deregisterSink(&testSink);

    REQUIRE(testSink.lastLogLevel == foeLogLevel::Verbose);
}

// Testing runtime log levels
FOE_DECLARE_LOG_CATEGORY(AllCategory, All, All)
FOE_DEFINE_LOG_CATEGORY(AllCategory)

TEST_CASE("foeLogCategory - Runtime log levels") {
    TestSink testSink;
    foeLogger::instance()->registerSink(&testSink);

    SECTION("Accepts down to Fatal") {
        AllCategory::instance()->maxLevel(foeLogLevel::Fatal);

        FOE_LOG(AllCategory, Fatal, "Fatal");
        FOE_LOG(AllCategory, Error, "Error");
        FOE_LOG(AllCategory, Warning, "Warning");
        FOE_LOG(AllCategory, Info, "Info");
        FOE_LOG(AllCategory, Verbose, "Verbose");

        REQUIRE(testSink.lastLogLevel == foeLogLevel::Fatal);
    }

    SECTION("Accepts down to Warning") {
        AllCategory::instance()->maxLevel(foeLogLevel::Warning);

        FOE_LOG(AllCategory, Fatal, "Fatal");
        FOE_LOG(AllCategory, Error, "Error");
        FOE_LOG(AllCategory, Warning, "Warning");
        FOE_LOG(AllCategory, Info, "Info");
        FOE_LOG(AllCategory, Verbose, "Verbose");

        REQUIRE(testSink.lastLogLevel == foeLogLevel::Warning);
    }

    SECTION("Accepts down to verbose") {
        AllCategory::instance()->maxLevel(foeLogLevel::Verbose);

        FOE_LOG(AllCategory, Fatal, "Fatal");
        FOE_LOG(AllCategory, Error, "Error");
        FOE_LOG(AllCategory, Warning, "Warning");
        FOE_LOG(AllCategory, Info, "Info");
        FOE_LOG(AllCategory, Verbose, "Verbose");

        REQUIRE(testSink.lastLogLevel == foeLogLevel::Verbose);
    }

    foeLogger::instance()->deregisterSink(&testSink);
}