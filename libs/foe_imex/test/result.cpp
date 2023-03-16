// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/imex/result.h>

#define ERROR_CODE_CATCH_CHECK(X)                                                                  \
    SECTION(#X) {                                                                                  \
        foeImexResultToString(X, resultString);                                                    \
        CHECK(std::string{resultString} == #X);                                                    \
    }

TEST_CASE("foeImexResult - Ensure error codes return correct values and strings") {
    char resultString[FOE_MAX_RESULT_STRING_SIZE];

    SECTION("Generic non-existant negative value") {
        foeImexResultToString((foeImexResult)FOE_RESULT_MIN_ENUM, resultString);
        CHECK(std::string{resultString} == "FOE_IMEX_UNKNOWN_ERROR_2147483647");
    }
    SECTION("Generic non-existant positive value") {
        foeImexResultToString((foeImexResult)FOE_RESULT_MAX_ENUM, resultString);
        CHECK(std::string{resultString} == "FOE_IMEX_UNKNOWN_SUCCESS_2147483647");
    }

    ERROR_CODE_CATCH_CHECK(FOE_IMEX_SUCCESS)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_ERROR_INCOMPLETE)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_ERROR_FUNCTION_NOT_DEFINED)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_ERROR_STRUCTURE_NOT_FOUND)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_ERROR_FUNCTIONALITY_ALREADY_REGISTERED)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_ERROR_EXPORTER_ALREADY_REGISTERED)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_ERROR_EXPORTER_NOT_REGISTERED)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_ERROR_IMPORTER_ALREADY_REGISTERED)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_ERROR_IMPORTER_NOT_REGISTERED)
}
