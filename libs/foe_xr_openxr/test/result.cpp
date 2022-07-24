// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/xr/openxr/result.h>

#define ERROR_CODE_CATCH_CHECK(X)                                                                  \
    SECTION(#X) {                                                                                  \
        char buffer[FOE_MAX_RESULT_STRING_SIZE];                                                   \
        foeOpenXrResultToString(X, buffer);                                                        \
        CHECK(std::string_view{buffer} == #X);                                                     \
    }

TEST_CASE("Ensure error codes return correct values and strings") {
    char resultString[FOE_MAX_RESULT_STRING_SIZE];

    SECTION("Generic non-existant negative value") {
        foeOpenXrResultToString((foeOpenXrResult)FOE_RESULT_MIN_ENUM, resultString);
        CHECK(std::string_view{resultString} == "FOE_OPENXR_UNKNOWN_ERROR_2147483647");
    }
    SECTION("Generic non-existant positive value") {
        foeOpenXrResultToString((foeOpenXrResult)FOE_RESULT_MAX_ENUM, resultString);
        CHECK(std::string_view{resultString} == "FOE_OPENXR_UNKNOWN_SUCCESS_2147483647");
    }

    ERROR_CODE_CATCH_CHECK(FOE_OPENXR_SUCCESS)
    ERROR_CODE_CATCH_CHECK(FOE_OPENXR_INCOMPLETE)
}