// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/wsi/result.h>

#define ERROR_CODE_CATCH_CHECK(X)                                                                  \
    SECTION(#X) {                                                                                  \
        foeWsiResultToString(X, resultString);                                                     \
        CHECK(std::string{resultString} == #X);                                                    \
    }

TEST_CASE("Ensure error codes return correct values and strings") {
    char resultString[FOE_MAX_RESULT_STRING_SIZE];

    SECTION("Generic non-existant negative value") {
        foeWsiResultToString((foeWsiResult)FOE_RESULT_MIN_ENUM, resultString);
        CHECK(std::string{resultString} == "FOE_WSI_UNKNOWN_ERROR_2147483647");
    }
    SECTION("Generic non-existant positive value") {
        foeWsiResultToString((foeWsiResult)FOE_RESULT_MAX_ENUM, resultString);
        CHECK(std::string{resultString} == "FOE_WSI_UNKNOWN_SUCCESS_2147483647");
    }

    ERROR_CODE_CATCH_CHECK(FOE_WSI_SUCCESS)
    ERROR_CODE_CATCH_CHECK(FOE_WSI_ERROR_OUT_OF_MEMORY)
    ERROR_CODE_CATCH_CHECK(FOE_WSI_ERROR_FAILED_TO_INITIALIZE_BACKEND)
    ERROR_CODE_CATCH_CHECK(FOE_WSI_ERROR_FAILED_TO_CREATE_WINDOW)
    ERROR_CODE_CATCH_CHECK(FOE_WSI_ERROR_VULKAN_NOT_SUPPORTED)
}