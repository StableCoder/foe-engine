/*
    Copyright (C) 2021 George Cave.

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
#include <foe/error_code.h>

#include <system_error>

enum TestResult { TEST_SUCCESS = 0, TEST_ERROR };

namespace {

struct TestErrCategory : std::error_category {
    const char *name() const noexcept override;
    std::string message(int ev) const override;
};

const char *TestErrCategory::name() const noexcept { return "TestResult"; }

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        return #X;

std::string TestErrCategory::message(int ev) const {
    switch (static_cast<TestResult>(ev)) {
        RESULT_CASE(TEST_SUCCESS)
        RESULT_CASE(TEST_ERROR)

    default:
        if (ev > 0)
            return "(unrecognized positive TestResult value)";
        else
            return "(unrecognized negative TestResult value)";
    }
}

#undef RESULT_CASE

const TestErrCategory errorCategory{};

} // namespace

namespace std {
template <>
struct is_error_code_enum<TestResult> : true_type {};
} // namespace std

std::error_code make_error_code(TestResult e) { return {static_cast<int>(e), errorCategory}; }

TEST_CASE("Regular std::error_code operates as expected") {
    SECTION("TEST_SUCCESS") {
        std::error_code errC = TEST_SUCCESS;

        CHECK(!errC);
        CHECK(errC.value() == TEST_SUCCESS);
        CHECK(errC.message() == "TEST_SUCCESS");
    }
    SECTION("TEST_ERROR") {
        std::error_code errC = TEST_ERROR;

        CHECK(errC);
        CHECK(errC.value() == TEST_ERROR);
        CHECK(errC.message() == "TEST_ERROR");
    }
}

TEST_CASE("foeErrorCode converted to std::error_code operates as expected") {
    SECTION("TEST_SUCCESS") {
        foeErrorCode test{TEST_SUCCESS, &errorCategory};
        std::error_code errC = test;

        CHECK(!errC);
        CHECK(errC.value() == TEST_SUCCESS);
        CHECK(errC.message() == "TEST_SUCCESS");
    }
    SECTION("TEST_ERROR") {
        foeErrorCode test{TEST_ERROR, &errorCategory};
        std::error_code errC = test;

        CHECK(errC);
        CHECK(errC.value() == TEST_ERROR);
        CHECK(errC.message() == "TEST_ERROR");
    }
}

TEST_CASE("Converting a std::error_code to foeErrorCode and back operates as expected") {
    SECTION("TEST_SUCCESS") {
        std::error_code startErrC = TEST_SUCCESS;
        foeErrorCode test{startErrC};
        std::error_code errC = test;

        CHECK(!errC);
        CHECK(errC.value() == TEST_SUCCESS);
        CHECK(errC.message() == "TEST_SUCCESS");
    }
    SECTION("TEST_ERROR") {
        std::error_code startErrC = TEST_ERROR;
        foeErrorCode test{startErrC};
        std::error_code errC = test;

        CHECK(errC);
        CHECK(errC.value() == TEST_ERROR);
        CHECK(errC.message() == "TEST_ERROR");
    }
}