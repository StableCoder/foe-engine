// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/crypto/result.h>
#include <foe/crypto/x25519.h>

#include <memory>
#include <string_view>

namespace {

constexpr std::string_view cOriginalData =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut "
    "labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco "
    "laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in "
    "voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat "
    "cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";

constexpr std::string_view invalidKeyData =
    "12345678901234567890123456789012345678901234567890123456789012345";

static_assert(
    invalidKeyData.size() >= FOE_CRYPTO_X25519_KEY_SIZE + 1,
    "Test key data must be at least 1 byte larger than the size of an ed25519 exchange key");

} // namespace

TEST_CASE("Create X25519 Keys") {
    foeCryptoKey privateKey = FOE_NULL_HANDLE;
    foeCryptoKey publicKey = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeCryptoCreateKeyPairX25519(&privateKey, &publicKey);
    REQUIRE(result.value == FOE_SUCCESS);

    REQUIRE(privateKey != FOE_NULL_HANDLE);
    CHECK(foeCryptoGetKeySize(privateKey) == FOE_CRYPTO_X25519_KEY_SIZE);
    CHECK(foeCryptoGetKeyData(privateKey) != nullptr);

    REQUIRE(publicKey != FOE_NULL_HANDLE);
    CHECK(foeCryptoGetKeySize(publicKey) == FOE_CRYPTO_X25519_KEY_SIZE);
    CHECK(foeCryptoGetKeyData(publicKey) != nullptr);

    foeDestroyCryptoKey(publicKey);
    foeDestroyCryptoKey(privateKey);
}

TEST_CASE("Perform X25519 Key Exchange") {
    foeCryptoKey serverPrivateKey = FOE_NULL_HANDLE;
    foeCryptoKey serverPublicKey = FOE_NULL_HANDLE;
    foeCryptoKey clientPrivateKey = FOE_NULL_HANDLE;
    foeCryptoKey clientPublicKey = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeCryptoCreateKeyPairX25519(&serverPrivateKey, &serverPublicKey);
    REQUIRE(result.value == FOE_SUCCESS);

    result = foeCryptoCreateKeyPairX25519(&clientPrivateKey, &clientPublicKey);
    REQUIRE(result.value == FOE_SUCCESS);

    SECTION("Perform key exchange both ways, ensure same key generated successfully") {
        foeCryptoKey sharedKey1 = FOE_NULL_HANDLE;
        foeCryptoKey sharedKey2 = FOE_NULL_HANDLE;

        result = foeCryptoPerformKeyExchangeX25519(serverPrivateKey, clientPublicKey,
                                                   clientPublicKey, serverPublicKey, &sharedKey1);
        REQUIRE(result.value == FOE_SUCCESS);
        REQUIRE(sharedKey1 != FOE_NULL_HANDLE);
        CHECK(foeCryptoGetKeySize(sharedKey1) == FOE_CRYPTO_X25519_KEY_SIZE);

        result = foeCryptoPerformKeyExchangeX25519(clientPrivateKey, serverPublicKey,
                                                   clientPublicKey, serverPublicKey, &sharedKey2);
        REQUIRE(result.value == FOE_SUCCESS);
        REQUIRE(sharedKey2 != FOE_NULL_HANDLE);
        CHECK(foeCryptoGetKeySize(sharedKey2) == FOE_CRYPTO_X25519_KEY_SIZE);

        REQUIRE(foeCryptoGetKeySize(sharedKey1) == foeCryptoGetKeySize(sharedKey2));

        // Ensure the two generated shared keys are identical
        REQUIRE(memcmp(foeCryptoGetKeyData(sharedKey1), foeCryptoGetKeyData(sharedKey2),
                       foeCryptoGetKeySize(sharedKey1)) == 0);

        foeDestroyCryptoKey(sharedKey2);
        foeDestroyCryptoKey(sharedKey1);
    }
    SECTION("Provided with invalid sized keys, key exchange fails") {
        foeCryptoKey smallerKey = FOE_NULL_HANDLE;
        foeCryptoKey largerKey = FOE_NULL_HANDLE;
        foeCryptoKey sharedKey = FOE_NULL_HANDLE;

        result =
            foeCreateCryptoKey(FOE_CRYPTO_X25519_KEY_SIZE - 1, invalidKeyData.data(), &smallerKey);
        REQUIRE(result.value == FOE_SUCCESS);

        result =
            foeCreateCryptoKey(FOE_CRYPTO_X25519_KEY_SIZE + 1, invalidKeyData.data(), &largerKey);
        REQUIRE(result.value == FOE_SUCCESS);

        SECTION("private key is smaller") {
            result = foeCryptoPerformKeyExchangeX25519(smallerKey, clientPublicKey, clientPublicKey,
                                                       serverPublicKey, &sharedKey);
            REQUIRE(result.value == FOE_CRYPTO_ERROR_INVALID_KEY_SIZE);
        }
        SECTION("public key is smaller") {
            result = foeCryptoPerformKeyExchangeX25519(
                serverPrivateKey, smallerKey, clientPublicKey, serverPublicKey, &sharedKey);
            REQUIRE(result.value == FOE_CRYPTO_ERROR_INVALID_KEY_SIZE);
        }
        SECTION("hash key 1 is smaller") {
            result = foeCryptoPerformKeyExchangeX25519(serverPrivateKey, clientPublicKey,
                                                       smallerKey, serverPublicKey, &sharedKey);
            REQUIRE(result.value == FOE_CRYPTO_ERROR_INVALID_KEY_SIZE);
        }
        SECTION("hash key 2 is smaller") {
            result = foeCryptoPerformKeyExchangeX25519(serverPrivateKey, clientPublicKey,
                                                       clientPublicKey, smallerKey, &sharedKey);
            REQUIRE(result.value == FOE_CRYPTO_ERROR_INVALID_KEY_SIZE);
        }

        SECTION("private key is larger") {
            result = foeCryptoPerformKeyExchangeX25519(largerKey, serverPrivateKey, clientPublicKey,
                                                       serverPublicKey, &sharedKey);
            REQUIRE(result.value == FOE_CRYPTO_ERROR_INVALID_KEY_SIZE);
        }
        SECTION("public key is larger") {
            result = foeCryptoPerformKeyExchangeX25519(serverPrivateKey, largerKey, clientPublicKey,
                                                       serverPublicKey, &sharedKey);
            REQUIRE(result.value == FOE_CRYPTO_ERROR_INVALID_KEY_SIZE);
        }
        SECTION("hash key 1 is larger") {
            result = foeCryptoPerformKeyExchangeX25519(serverPrivateKey, clientPublicKey, largerKey,
                                                       serverPublicKey, &sharedKey);
            REQUIRE(result.value == FOE_CRYPTO_ERROR_INVALID_KEY_SIZE);
        }
        SECTION("hash key 2 is larger") {
            result = foeCryptoPerformKeyExchangeX25519(serverPrivateKey, clientPublicKey,
                                                       clientPublicKey, largerKey, &sharedKey);
            REQUIRE(result.value == FOE_CRYPTO_ERROR_INVALID_KEY_SIZE);
        }
    }

    foeDestroyCryptoKey(clientPublicKey);
    foeDestroyCryptoKey(clientPrivateKey);
    foeDestroyCryptoKey(serverPublicKey);
    foeDestroyCryptoKey(serverPrivateKey);
}