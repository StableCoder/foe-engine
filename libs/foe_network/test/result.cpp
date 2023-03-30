// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/network/result.h>

#define ERROR_CODE_CATCH_CHECK(X)                                                                  \
    SECTION(#X) {                                                                                  \
        foeNetworkResultToString(X, resultString);                                                 \
        CHECK(std::string{resultString} == #X);                                                    \
    }

TEST_CASE("foeNetworkResult - Ensure error codes return correct values and strings") {
    char resultString[FOE_MAX_RESULT_STRING_SIZE];

    SECTION("Generic non-existant negative value") {
        foeNetworkResultToString((foeNetworkResult)FOE_RESULT_MIN_ENUM, resultString);
        CHECK(std::string{resultString} == "FOE_NETWORK_UNKNOWN_ERROR_2147483647");
    }
    SECTION("Generic non-existant positive value") {
        foeNetworkResultToString((foeNetworkResult)FOE_RESULT_MAX_ENUM, resultString);
        CHECK(std::string{resultString} == "FOE_NETWORK_UNKNOWN_SUCCESS_2147483647");
    }

    ERROR_CODE_CATCH_CHECK(FOE_NETWORK_SUCCESS)
    ERROR_CODE_CATCH_CHECK(FOE_NETWORK_INCOMPLETE)
    ERROR_CODE_CATCH_CHECK(FOE_NETWORK_STRING_INCOMPLETE)
    ERROR_CODE_CATCH_CHECK(FOE_NETWORK_ERROR_INVALID_PORT)
    ERROR_CODE_CATCH_CHECK(FOE_NETWORK_ERROR_NOT_IPV6)
    ERROR_CODE_CATCH_CHECK(FOE_NETWORK_ERROR_SOCKET_CREATION_ERROR)
    ERROR_CODE_CATCH_CHECK(FOE_NETWORK_ERROR_SOCKET_FAILED_TO_SET_DUAL_STACK)
    ERROR_CODE_CATCH_CHECK(FOE_NETWORK_ERROR_SOCKET_BIND_ERROR)
    ERROR_CODE_CATCH_CHECK(FOE_NETWORK_ERROR_WSA_STARTUP_FAILED)
    ERROR_CODE_CATCH_CHECK(FOE_NETWORK_ERROR_WSA_INCORRECT_VERSION)
    ERROR_CODE_CATCH_CHECK(FOE_NETWORK_ERROR_SEND_FAILURE)
    ERROR_CODE_CATCH_CHECK(FOE_NETWORK_ERROR_PARTIAL_SEND)
    ERROR_CODE_CATCH_CHECK(FOE_NETWORK_ERROR_RECIEVE_FAILURE)
    ERROR_CODE_CATCH_CHECK(FOE_NETWORK_ERROR_UNSUPPORTED_FAMILY_TYPE)
    ERROR_CODE_CATCH_CHECK(FOE_NETWORK_ERROR_SYSTEM_CALL_FAILED)
}
