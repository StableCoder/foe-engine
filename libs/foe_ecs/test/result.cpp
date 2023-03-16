// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/ecs/result.h>

#define ERROR_CODE_CATCH_CHECK(X)                                                                  \
    SECTION(#X) {                                                                                  \
        foeEcsResultToString(X, resultString);                                                     \
        CHECK(std::string{resultString} == #X);                                                    \
    }

TEST_CASE("foeEcsResult - Ensure error codes return correct values and strings") {
    char resultString[FOE_MAX_RESULT_STRING_SIZE];

    SECTION("Generic non-existant negative value") {
        foeEcsResultToString((foeEcsResult)FOE_RESULT_MIN_ENUM, resultString);
        CHECK(std::string{resultString} == "FOE_ECS_UNKNOWN_ERROR_2147483647");
    }
    SECTION("Generic non-existant positive value") {
        foeEcsResultToString((foeEcsResult)FOE_RESULT_MAX_ENUM, resultString);
        CHECK(std::string{resultString} == "FOE_ECS_UNKNOWN_SUCCESS_2147483647");
    }

    ERROR_CODE_CATCH_CHECK(FOE_ECS_SUCCESS)
    ERROR_CODE_CATCH_CHECK(FOE_ECS_INCOMPLETE)
    ERROR_CODE_CATCH_CHECK(FOE_ECS_NO_MATCH)
    ERROR_CODE_CATCH_CHECK(FOE_ECS_ERROR_OUT_OF_MEMORY)
    ERROR_CODE_CATCH_CHECK(FOE_ECS_ERROR_INDEX_BELOW_MINIMUM)
    ERROR_CODE_CATCH_CHECK(FOE_ECS_ERROR_NO_MATCHING_GROUP)
    ERROR_CODE_CATCH_CHECK(FOE_ECS_ERROR_NOT_GROUP_ID)
    ERROR_CODE_CATCH_CHECK(FOE_ECS_ERROR_OUT_OF_INDEXES)
    ERROR_CODE_CATCH_CHECK(FOE_ECS_ERROR_INVALID_ID)
    ERROR_CODE_CATCH_CHECK(FOE_ECS_ERROR_INCORRECT_GROUP_ID)
    ERROR_CODE_CATCH_CHECK(FOE_ECS_ERROR_INDEX_ABOVE_GENERATED)
    ERROR_CODE_CATCH_CHECK(FOE_ECS_ERROR_EMPTY_NAME)
    ERROR_CODE_CATCH_CHECK(FOE_ECS_ERROR_ID_ALREADY_EXISTS)
    ERROR_CODE_CATCH_CHECK(FOE_ECS_ERROR_NAME_ALREADY_EXISTS)
    ERROR_CODE_CATCH_CHECK(FOE_ECS_ERROR_NO_MATCH)
}
