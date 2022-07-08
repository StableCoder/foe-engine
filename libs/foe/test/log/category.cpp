// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/log.hpp>

FOE_DECLARE_LOG_CATEGORY(TestCategory, All, All)
FOE_DEFINE_LOG_CATEGORY(TestCategory)

TEST_CASE("foeLogCategory Functionality", "[foe][log]") {
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

TEST_CASE("foeLogCategory - Fatal compilation level", "[foe][log]") {
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

TEST_CASE("foeLogCategory - Warning compilation level", "[foe][log]") {
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

TEST_CASE("foeLogCategory - Verbose compilation level", "[foe][log]") {
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

TEST_CASE("foeLogCategory - Runtime log levels", "[foe][log]") {
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