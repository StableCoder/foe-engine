// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/crypto/sha256.h>
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
    "2d8c2f6d978ca21712b5f6de36c9d31fa8e96a4fa5d8ff8b0188dfb9e7c171bb";

} // namespace

TEST_CASE("SHA256 Hashing") {
    uint8_t calculatedHash[32];
    memset(calculatedHash, 0, sizeof(calculatedHash));

    CHECK(foeCryptoHashSHA256(cTestText.size(), cTestText.data(), calculatedHash));

    uint8_t expectedHash[32];
    size_t decodedSize = 32;
    foeResultSet result =
        foeDecodeHex(cExpectedHash.size(), cExpectedHash.data(), &decodedSize, expectedHash);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(decodedSize == 32);

    CHECK(memcmp(calculatedHash, expectedHash, 32) == 0);
}