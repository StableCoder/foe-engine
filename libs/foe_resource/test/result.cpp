// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/resource/result.h>

#define ERROR_CODE_CATCH_CHECK(X)                                                                  \
    SECTION(#X) {                                                                                  \
        foeResourceResultToString(X, resultString);                                                \
        CHECK(std::string{resultString} == #X);                                                    \
    }

TEST_CASE("foeResourceResult - Ensure error codes return correct values and strings") {
    char resultString[FOE_MAX_RESULT_STRING_SIZE];

    SECTION("Generic non-existant negative value") {
        foeResourceResultToString((foeResourceResult)FOE_RESULT_MIN_ENUM, resultString);
        CHECK(std::string{resultString} == "FOE_RESOURCE_UNKNOWN_ERROR_2147483647");
    }
    SECTION("Generic non-existant positive value") {
        foeResourceResultToString((foeResourceResult)FOE_RESULT_MAX_ENUM, resultString);
        CHECK(std::string{resultString} == "FOE_RESOURCE_UNKNOWN_SUCCESS_2147483647");
    }

    ERROR_CODE_CATCH_CHECK(FOE_RESOURCE_SUCCESS)
    ERROR_CODE_CATCH_CHECK(FOE_RESOURCE_NO_MODIFIED_RECORD)
    ERROR_CODE_CATCH_CHECK(FOE_RESOURCE_ALREADY_LOADING)
    ERROR_CODE_CATCH_CHECK(FOE_RESOURCE_ERROR_NOT_FOUND)
    ERROR_CODE_CATCH_CHECK(FOE_RESOURCE_ERROR_OUT_OF_MEMORY)
    ERROR_CODE_CATCH_CHECK(FOE_RESOURCE_ERROR_RESOURCE_FUNCTIONS_NOT_PROVIDED)
    ERROR_CODE_CATCH_CHECK(FOE_RESOURCE_ERROR_DATA_FUNCTION_NOT_PROVIDED)
    ERROR_CODE_CATCH_CHECK(FOE_RESOURCE_ERROR_DATA_SIZE_SMALLER_THAN_BASE)
    ERROR_CODE_CATCH_CHECK(FOE_RESOURCE_ERROR_RESOURCE_NOT_UNDEFINED)
    ERROR_CODE_CATCH_CHECK(FOE_RESOURCE_ERROR_REPLACED_CANNOT_BE_LOADED)
}
