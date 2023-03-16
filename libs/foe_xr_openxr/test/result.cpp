// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/xr/openxr/result.h>

#define ERROR_CODE_CATCH_CHECK(X)                                                                  \
    SECTION(#X) {                                                                                  \
        foeOpenXrResultToString(X, resultString);                                                  \
        CHECK(std::string{resultString} == #X);                                                    \
    }

TEST_CASE("foeOpenXrResult - Ensure error codes return correct values and strings") {
    char resultString[FOE_MAX_RESULT_STRING_SIZE];

    SECTION("Generic non-existant negative value") {
        foeOpenXrResultToString((foeOpenXrResult)FOE_RESULT_MIN_ENUM, resultString);
        CHECK(std::string{resultString} == "FOE_OPEN_XR_UNKNOWN_ERROR_2147483647");
    }
    SECTION("Generic non-existant positive value") {
        foeOpenXrResultToString((foeOpenXrResult)FOE_RESULT_MAX_ENUM, resultString);
        CHECK(std::string{resultString} == "FOE_OPEN_XR_UNKNOWN_SUCCESS_2147483647");
    }

    ERROR_CODE_CATCH_CHECK(FOE_OPENXR_SUCCESS)
    ERROR_CODE_CATCH_CHECK(FOE_OPENXR_INCOMPLETE)
    ERROR_CODE_CATCH_CHECK(FOE_OPENXR_ERROR_OUT_OF_MEMORY)
}
