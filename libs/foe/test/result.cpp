// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/result.h>

#define ERROR_CODE_CATCH_CHECK(X)                                                                  \
    SECTION(#X) {                                                                                  \
        foeResultToString(X, resultString);                                                        \
        CHECK(std::string_view{resultString} == #X);                                               \
    }

TEST_CASE("foeResult - Ensure error codes return correct values and strings") {
    char resultString[FOE_MAX_RESULT_STRING_SIZE];

    SECTION("Generic non-existant negative value") {
        foeResultToString((foeResult)FOE_RESULT_MIN_ENUM, resultString);
        CHECK(std::string_view{resultString} == "FOE_UNKNOWN_ERROR_2147483647");
    }
    SECTION("Generic non-existant positive value") {
        foeResultToString((foeResult)FOE_RESULT_MAX_ENUM, resultString);
        CHECK(std::string_view{resultString} == "FOE_UNKNOWN_SUCCESS_2147483647");
    }

    ERROR_CODE_CATCH_CHECK(FOE_SUCCESS)
    ERROR_CODE_CATCH_CHECK(FOE_ERROR_OUT_OF_MEMORY)
    ERROR_CODE_CATCH_CHECK(FOE_ERROR_FAILED_TO_OPEN_FILE)
    ERROR_CODE_CATCH_CHECK(FOE_ERROR_FAILED_TO_STAT_FILE)
    ERROR_CODE_CATCH_CHECK(FOE_ERROR_ATTEMPTED_TO_MAP_ZERO_SIZED_FILE)
    ERROR_CODE_CATCH_CHECK(FOE_ERROR_FAILED_TO_MAP_FILE)
    ERROR_CODE_CATCH_CHECK(FOE_ERROR_FAILED_TO_UNMAP_FILE)
    ERROR_CODE_CATCH_CHECK(FOE_ERROR_FAILED_TO_CLOSE_FILE)
    ERROR_CODE_CATCH_CHECK(FOE_ERROR_MEMORY_SUBSET_OVERRUNS_PARENT)
}
