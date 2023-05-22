// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/crypto/key.h>
#include <foe/crypto/result.h>

namespace {

constexpr std::string_view keyData = "1234567890";

}

TEST_CASE("foeCryptoKey - Creating a key successfully has the expected key size and data") {
    foeCryptoKey key = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeCreateCryptoKey(keyData.size(), keyData.data(), &key);
    REQUIRE(result.value == FOE_SUCCESS);

    REQUIRE(key != FOE_NULL_HANDLE);
    REQUIRE(foeCryptoGetKeySize(key) == keyData.size());
    REQUIRE(foeCryptoGetKeyData(key) != nullptr);
    REQUIRE(memcmp(keyData.data(), foeCryptoGetKeyData(key), keyData.size()) == 0);

    foeDestroyCryptoKey(key);
}

TEST_CASE("foeCryptoKey - Creating a key with a zero-size fails") {
    foeCryptoKey key = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeCreateCryptoKey(0, nullptr, &key);

    REQUIRE(result.value == FOE_CRYPTO_ERROR_INVALID_KEY_SIZE);
    REQUIRE(key == FOE_NULL_HANDLE);
}