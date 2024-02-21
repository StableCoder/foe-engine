// Copyright (C) 2023-2024 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/crypto/ed25519.h>
#include <foe/crypto/result.h>

#include <cstring>
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
    invalidKeyData.size() >= FOE_CRYPTO_ED25519_PRIVATE_KEY_SIZE + 1,
    "Test key data must be at least 1 byte larger than the size of an ed25519 private signing key");
static_assert(
    invalidKeyData.size() >= FOE_CRYPTO_ED25519_PUBLIC_KEY_SIZE + 1,
    "Test key data must be at least 1 byte larger than the size of an ed25519 public signing key");

} // namespace

TEST_CASE("Create ED25519 Keys") {
    foeCryptoKey privateKey = FOE_NULL_HANDLE;
    foeCryptoKey publicKey = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeCryptoCreateKeyPairED25519(&privateKey, &publicKey);
    REQUIRE(result.value == FOE_SUCCESS);

    REQUIRE(privateKey != FOE_NULL_HANDLE);
    REQUIRE(foeCryptoGetKeySize(privateKey) == FOE_CRYPTO_ED25519_PRIVATE_KEY_SIZE);
    REQUIRE(foeCryptoGetKeyData(privateKey) != nullptr);

    REQUIRE(publicKey != FOE_NULL_HANDLE);
    REQUIRE(foeCryptoGetKeySize(publicKey) == FOE_CRYPTO_ED25519_PUBLIC_KEY_SIZE);
    REQUIRE(foeCryptoGetKeyData(publicKey) != nullptr);

    SECTION("Re-create public key from private key") {
        foeCryptoKey recreatedPublicKey = FOE_NULL_HANDLE;

        result = foeCryptoCreatePublicKeyED25519(privateKey, &recreatedPublicKey);
        REQUIRE(result.value == FOE_SUCCESS);

        REQUIRE(foeCryptoGetKeySize(recreatedPublicKey) == FOE_CRYPTO_ED25519_PUBLIC_KEY_SIZE);
        REQUIRE(foeCryptoGetKeyData(recreatedPublicKey) != nullptr);

        CHECK(memcmp(foeCryptoGetKeyData(publicKey), foeCryptoGetKeyData(recreatedPublicKey),
                     FOE_CRYPTO_ED25519_PUBLIC_KEY_SIZE) == 0);
    }

    foeDestroyCryptoKey(publicKey);
    foeDestroyCryptoKey(privateKey);
}

TEST_CASE("ED25519 successful Sign/Verify") {
    foeCryptoKey privateKey = FOE_NULL_HANDLE;
    foeCryptoKey publicKey = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeCryptoCreateKeyPairED25519(&privateKey, &publicKey);
    REQUIRE(result.value == FOE_SUCCESS);

    // Perform Signing
    uint8_t signatureBuffer[FOE_CRYPTO_ED25519_SIGNATURE_SIZE];
    memset(signatureBuffer, 0, sizeof(signatureBuffer));

    result = foeCryptoSignDataED25519(privateKey, cOriginalData.size(), cOriginalData.data(),
                                      signatureBuffer);
    REQUIRE(result.value == FOE_SUCCESS);

    // Perform Verify
    SECTION("Verifies with correct public key") {
        result = foeCryptoVerifyDataED25519(publicKey, cOriginalData.size(), cOriginalData.data(),
                                            signatureBuffer);
        REQUIRE(result.value == FOE_SUCCESS);
    }
    SECTION("Failed verify with incorrect public key") {
        foeCryptoKey invalidKey = FOE_NULL_HANDLE;

        result = foeCreateCryptoKey(FOE_CRYPTO_ED25519_PUBLIC_KEY_SIZE, invalidKeyData.data(),
                                    &invalidKey);
        REQUIRE(result.value == FOE_SUCCESS);

        result = foeCryptoVerifyDataED25519(invalidKey, cOriginalData.size(), cOriginalData.data(),
                                            signatureBuffer);
        REQUIRE(result.value == FOE_CRYPTO_ERROR_FAILED_TO_VERIFY);

        foeDestroyCryptoKey(invalidKey);
    }

    foeDestroyCryptoKey(publicKey);
    foeDestroyCryptoKey(privateKey);
}

TEST_CASE("ED25519 Signing/Verifying fails with invalid key sizes") {
    uint8_t signatureBuffer[FOE_CRYPTO_ED25519_SIGNATURE_SIZE];
    foeCryptoKey invalidKey = FOE_NULL_HANDLE;
    foeResultSet result;

    SECTION("Invalid signing key sizes") {
        SECTION("Smaller key") {
            result = foeCreateCryptoKey(FOE_CRYPTO_ED25519_PRIVATE_KEY_SIZE - 1,
                                        invalidKeyData.data(), &invalidKey);
            REQUIRE(result.value == FOE_SUCCESS);

            result = foeCryptoSignDataED25519(invalidKey, cOriginalData.size(),
                                              cOriginalData.data(), signatureBuffer);
            REQUIRE(result.value == FOE_CRYPTO_ERROR_INVALID_KEY_SIZE);
        }
        SECTION("Larger key") {
            result = foeCreateCryptoKey(FOE_CRYPTO_ED25519_PRIVATE_KEY_SIZE + 1,
                                        invalidKeyData.data(), &invalidKey);
            REQUIRE(result.value == FOE_SUCCESS);

            result = foeCryptoSignDataED25519(invalidKey, cOriginalData.size(),
                                              cOriginalData.data(), signatureBuffer);
            REQUIRE(result.value == FOE_CRYPTO_ERROR_INVALID_KEY_SIZE);
        }
    }

    SECTION("Invalid verify key sizes") {
        SECTION("Smaller key") {
            result = foeCreateCryptoKey(FOE_CRYPTO_ED25519_PUBLIC_KEY_SIZE - 1,
                                        invalidKeyData.data(), &invalidKey);
            REQUIRE(result.value == FOE_SUCCESS);

            result = foeCryptoVerifyDataED25519(invalidKey, cOriginalData.size(),
                                                cOriginalData.data(), signatureBuffer);
            REQUIRE(result.value == FOE_CRYPTO_ERROR_INVALID_KEY_SIZE);
        }
        SECTION("Larger key") {
            result = foeCreateCryptoKey(FOE_CRYPTO_ED25519_PUBLIC_KEY_SIZE + 1,
                                        invalidKeyData.data(), &invalidKey);
            REQUIRE(result.value == FOE_SUCCESS);

            result = foeCryptoVerifyDataED25519(invalidKey, cOriginalData.size(),
                                                cOriginalData.data(), signatureBuffer);
            REQUIRE(result.value == FOE_CRYPTO_ERROR_INVALID_KEY_SIZE);
        }
    }

    foeDestroyCryptoKey(invalidKey);
}