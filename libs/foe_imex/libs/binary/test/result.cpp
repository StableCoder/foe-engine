// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/imex/binary/result.h>

#define ERROR_CODE_CATCH_CHECK(X)                                                                  \
    SECTION(#X) {                                                                                  \
        foeImexBinaryResultToString(X, resultString);                                              \
        CHECK(std::string_view{resultString} == #X);                                               \
    }

TEST_CASE("foeImexBinaryResult - Ensure error codes return correct values and strings") {
    char resultString[FOE_MAX_RESULT_STRING_SIZE];

    SECTION("Generic non-existant negative value") {
        foeImexBinaryResultToString((foeImexBinaryResult)FOE_RESULT_MIN_ENUM, resultString);
        CHECK(std::string_view{resultString} == "FOE_IMEX_BINARY_UNKNOWN_ERROR_2147483647");
    }
    SECTION("Generic non-existant positive value") {
        foeImexBinaryResultToString((foeImexBinaryResult)FOE_RESULT_MAX_ENUM, resultString);
        CHECK(std::string_view{resultString} == "FOE_IMEX_BINARY_UNKNOWN_SUCCESS_2147483647");
    }

    ERROR_CODE_CATCH_CHECK(FOE_IMEX_BINARY_SUCCESS)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_BINARY_INCOMPLETE)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_BINARY_ERROR_OUT_OF_MEMORY)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_BINARY_ERROR_FUNCTIONALITY_ALREADY_REGISTERED)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_BINARY_ERROR_FUNCTIONALITY_NOT_REGISTERED)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_BINARY_ERROR_DESTINATION_NOT_FILE)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_BINARY_ERROR_FAILED_TO_EXPORT_DEPENDENCIES)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_BINARY_ERROR_FAILED_TO_EXPORT_RESOURCE)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_BINARY_ERROR_KEY_ALREADY_REGISTERED)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_BINARY_ERROR_KEY_NOT_REGISTERED)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_BINARY_ERROR_KEY_FUNCTIONS_NON_MATCHING)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_BINARY_ERROR_FILE_NOT_EXIST)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_BINARY_ERROR_NOT_REGULAR_FILE)
}
