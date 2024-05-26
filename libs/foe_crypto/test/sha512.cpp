// Copyright (C) 2023-2024 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/crypto/sha512.h>
#include <foe/hex.h>

#include <cstring>
#include <string_view>

namespace {

constexpr std::string_view cTestText =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut "
    "labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco "
    "laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in "
    "voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat "
    "cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";

constexpr std::string_view cExpectedHash =
    "8ba760cac29cb2b2ce66858ead169174057aa1298ccd581514e6db6dee3285280ee6e3a54c9319071dc8165ff061d7"
    "7783100d449c937ff1fb4cd1bb516a69b9";

} // namespace

TEST_CASE("SHA512 Hashing") {
    uint8_t calculatedHash[FOE_CRYPTO_SHA512_HASH_SIZE];
    memset(calculatedHash, 0, sizeof(calculatedHash));

    CHECK(foeCryptoHashSHA512(cTestText.size(), cTestText.data(), calculatedHash));

    uint8_t expectedHash[FOE_CRYPTO_SHA512_HASH_SIZE];
    size_t decodedSize = FOE_CRYPTO_SHA512_HASH_SIZE;
    foeResultSet result =
        foeDecodeHex(cExpectedHash.size(), cExpectedHash.data(), &decodedSize, expectedHash);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(decodedSize == FOE_CRYPTO_SHA512_HASH_SIZE);

    CHECK(memcmp(calculatedHash, expectedHash, FOE_CRYPTO_SHA512_HASH_SIZE) == 0);
}