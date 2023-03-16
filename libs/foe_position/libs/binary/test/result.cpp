// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/position/binary/result.h>

#define ERROR_CODE_CATCH_CHECK(X)                                                                  \
    SECTION(#X) {                                                                                  \
        foePositionBinaryResultToString(X, resultString);                                          \
        CHECK(std::string{resultString} == #X);                                                    \
    }

TEST_CASE("foePositionBinaryResult - Ensure error codes return correct values and strings") {
    char resultString[FOE_MAX_RESULT_STRING_SIZE];

    SECTION("Generic non-existant negative value") {
        foePositionBinaryResultToString((foePositionBinaryResult)FOE_RESULT_MIN_ENUM, resultString);
        CHECK(std::string{resultString} == "FOE_POSITION_BINARY_UNKNOWN_ERROR_2147483647");
    }
    SECTION("Generic non-existant positive value") {
        foePositionBinaryResultToString((foePositionBinaryResult)FOE_RESULT_MAX_ENUM, resultString);
        CHECK(std::string{resultString} == "FOE_POSITION_BINARY_UNKNOWN_SUCCESS_2147483647");
    }

    ERROR_CODE_CATCH_CHECK(FOE_POSITION_BINARY_SUCCESS)
    ERROR_CODE_CATCH_CHECK(FOE_POSITION_BINARY_DATA_NOT_EXPORTED)
    ERROR_CODE_CATCH_CHECK(FOE_POSITION_BINARY_ERROR_OUT_OF_MEMORY)
    ERROR_CODE_CATCH_CHECK(FOE_POSITION_BINARY_ERROR_FAILED_TO_REGISTER_3D_IMPORTER)
    ERROR_CODE_CATCH_CHECK(FOE_POSITION_BINARY_ERROR_FAILED_TO_REGISTER_3D_EXPORTER)
    ERROR_CODE_CATCH_CHECK(FOE_POSITION_BINARY_ERROR_POSITION_3D_POOL_NOT_FOUND)
}
