// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/crypto/result.h>

#define ERROR_CODE_CATCH_CHECK(X)                                                                  \
    SECTION(#X) {                                                                                  \
        foeCryptoResultToString(X, resultString);                                                  \
        CHECK(std::string{resultString} == #X);                                                    \
    }

TEST_CASE("foeCryptoResult - Ensure error codes return correct values and strings") {
    char resultString[FOE_MAX_RESULT_STRING_SIZE];

    SECTION("Generic non-existant negative value") {
        foeCryptoResultToString((foeCryptoResult)FOE_RESULT_MIN_ENUM, resultString);
        CHECK(std::string{resultString} == "FOE_CRYPTO_UNKNOWN_ERROR_2147483647");
    }
    SECTION("Generic non-existant positive value") {
        foeCryptoResultToString((foeCryptoResult)FOE_RESULT_MAX_ENUM, resultString);
        CHECK(std::string{resultString} == "FOE_CRYPTO_UNKNOWN_SUCCESS_2147483647");
    }

    ERROR_CODE_CATCH_CHECK(FOE_CRYPTO_SUCCESS)
    ERROR_CODE_CATCH_CHECK(FOE_CRYPTO_ERROR_OUT_OF_MEMORY)
    ERROR_CODE_CATCH_CHECK(FOE_CRYPTO_ERROR_LIBRARY_FAILED_TO_INITIALIZE)
    ERROR_CODE_CATCH_CHECK(FOE_CRYPTO_ERROR_INITIALIZATION_FAILED)
    ERROR_CODE_CATCH_CHECK(FOE_CRYPTO_ERROR_NO_AES_HARDWARE_ACCELERATION_AVAILABLE)
    ERROR_CODE_CATCH_CHECK(FOE_CRYPTO_ERROR_INVALID_KEY_SIZE)
    ERROR_CODE_CATCH_CHECK(FOE_CRYPTO_ERROR_INVALID_NONCE_SIZE)
    ERROR_CODE_CATCH_CHECK(FOE_CRYPTO_ERROR_FAILED_TO_CREATE_KEY_PAIR)
    ERROR_CODE_CATCH_CHECK(FOE_CRYPTO_ERROR_FAILED_TO_PERFORM_KEY_EXCHANGE)
    ERROR_CODE_CATCH_CHECK(FOE_CRYPTO_ERROR_DESTINATION_BUFFER_TOO_SMALL)
    ERROR_CODE_CATCH_CHECK(FOE_CRYPTO_ERROR_FAILED_TO_SIGN)
    ERROR_CODE_CATCH_CHECK(FOE_CRYPTO_ERROR_FAILED_TO_VERIFY)
    ERROR_CODE_CATCH_CHECK(FOE_CRYPTO_ERROR_FAILED_TO_ENCRYPT)
    ERROR_CODE_CATCH_CHECK(FOE_CRYPTO_ERROR_FAILED_TO_DECRYPT)
}
