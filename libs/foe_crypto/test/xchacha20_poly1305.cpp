// Copyright (C) 2023-2024 George Cave
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/crypto/result.h>
#include <foe/crypto/xchacha20_poly1305.h>

#include <cstring>
#include <memory>

namespace {

constexpr std::string_view cOriginalData =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut "
    "labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco "
    "laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in "
    "voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat "
    "cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";

constexpr std::string_view validKeyData = "abcdefghijklmnopqrstuvwxyzabcdef";
static_assert(validKeyData.size() == FOE_CRYPTO_XCHACHA20_POLY1305_KEY_SIZE);

constexpr std::string_view invalidKeyData =
    "12345678901234567890123456789012345678901234567890123456789012345";

constexpr std::string_view nonce1 = "123456789012345678901234";
static_assert(nonce1.size() == FOE_CRYPTO_XCHACHA20_POLY1305_NONCE_SIZE);

constexpr std::string_view nonce2 = "432109876543210987654321";
static_assert(nonce2.size() == FOE_CRYPTO_XCHACHA20_POLY1305_NONCE_SIZE);

} // namespace

TEST_CASE("XChaCha20 Poly1305 - encryption success cases") {
    foeCryptoKey key = FOE_NULL_HANDLE;
    foeResultSet result;

    // Create valid key
    result = foeCreateCryptoKey(validKeyData.size(), validKeyData.data(), &key);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(key != FOE_NULL_HANDLE);

    // Encrypt data
    size_t encryptedBufferSize =
        cOriginalData.size() + FOE_CRYPTO_XCHACHA20_POLY1305_ENCRYPTION_OVERHEAD;
    std::unique_ptr<unsigned char[]> encryptedBuffer(new unsigned char[encryptedBufferSize + 16]);
    memset(encryptedBuffer.get(), 0, encryptedBufferSize + 16);

    size_t encryptedDataSize = encryptedBufferSize;
    SECTION("destination buffer size is exact required") {
        result = foeCryptoEncrypt_XChaCha20_Poly1305(key, nonce1.size(), nonce1.data(),
                                                     cOriginalData.size(), cOriginalData.data(),
                                                     &encryptedDataSize, encryptedBuffer.get());
        REQUIRE(result.value == FOE_SUCCESS);
        REQUIRE(encryptedDataSize == encryptedBufferSize);
    }
    SECTION("destination buffer size is too large") {
        encryptedDataSize += 16;
        result = foeCryptoEncrypt_XChaCha20_Poly1305(key, nonce1.size(), nonce1.data(),
                                                     cOriginalData.size(), cOriginalData.data(),
                                                     &encryptedDataSize, encryptedBuffer.get());
        REQUIRE(result.value == FOE_SUCCESS);
        REQUIRE(encryptedDataSize == encryptedBufferSize);
    }

    // Decrypt data
    constexpr size_t decryptedDataBufferSize = cOriginalData.size();
    std::unique_ptr<unsigned char[]> decryptedBuffer(new unsigned char[decryptedDataBufferSize]);

    size_t decryptedDataSize = decryptedDataBufferSize;
    result = foeCryptoDecrypt_XChaCha20_Poly1305(key, nonce1.size(), nonce1.data(),
                                                 encryptedDataSize, encryptedBuffer.get(),
                                                 &decryptedDataSize, decryptedBuffer.get());
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(decryptedDataSize == decryptedDataBufferSize);
    REQUIRE(memcmp(cOriginalData.data(), decryptedBuffer.get(), cOriginalData.size()) == 0);

    // Cleanup
    foeDestroyCryptoKey(key);
}

TEST_CASE("XChaCha20 Poly1305 - decryption success cases") {
    foeCryptoKey key = FOE_NULL_HANDLE;
    foeResultSet result;

    // Create valid key
    result = foeCreateCryptoKey(validKeyData.size(), validKeyData.data(), &key);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(key != FOE_NULL_HANDLE);

    // Encrypt data
    size_t encryptedBufferSize =
        cOriginalData.size() + FOE_CRYPTO_XCHACHA20_POLY1305_ENCRYPTION_OVERHEAD;
    std::unique_ptr<unsigned char[]> encryptedBuffer(new unsigned char[encryptedBufferSize + 16]);
    memset(encryptedBuffer.get(), 0, encryptedBufferSize + 16);

    size_t encryptedDataSize = encryptedBufferSize;
    result = foeCryptoEncrypt_XChaCha20_Poly1305(key, nonce1.size(), nonce1.data(),
                                                 cOriginalData.size(), cOriginalData.data(),
                                                 &encryptedDataSize, encryptedBuffer.get());
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(encryptedDataSize == encryptedBufferSize);

    // Decrypt data
    constexpr size_t decryptedDataBufferSize = cOriginalData.size();
    std::unique_ptr<unsigned char[]> decryptedBuffer(
        new unsigned char[decryptedDataBufferSize + 16]);

    size_t decryptedDataSize = decryptedDataBufferSize;
    SECTION("destination buffer size is exact required") {
        result = foeCryptoDecrypt_XChaCha20_Poly1305(key, nonce1.size(), nonce1.data(),
                                                     encryptedDataSize, encryptedBuffer.get(),
                                                     &decryptedDataSize, decryptedBuffer.get());
        REQUIRE(result.value == FOE_SUCCESS);
        REQUIRE(decryptedDataSize == decryptedDataBufferSize);
        REQUIRE(memcmp(cOriginalData.data(), decryptedBuffer.get(), cOriginalData.size()) == 0);
    }
    SECTION("destination buffer size is too large") {
        decryptedDataSize += 16;
        result = foeCryptoDecrypt_XChaCha20_Poly1305(key, nonce1.size(), nonce1.data(),
                                                     encryptedDataSize, encryptedBuffer.get(),
                                                     &decryptedDataSize, decryptedBuffer.get());
        REQUIRE(result.value == FOE_SUCCESS);
        REQUIRE(decryptedDataSize == decryptedDataBufferSize);
        REQUIRE(memcmp(cOriginalData.data(), decryptedBuffer.get(), cOriginalData.size()) == 0);
    }

    // Cleanup
    foeDestroyCryptoKey(key);
}

TEST_CASE("XChaCha20 Poly1305 - encryption failure cases") {
    foeResultSet result;

    size_t encryptedBufferSize =
        cOriginalData.size() + FOE_CRYPTO_XCHACHA20_POLY1305_ENCRYPTION_OVERHEAD;
    std::unique_ptr<unsigned char[]> encryptedBuffer(new unsigned char[encryptedBufferSize]);
    memset(encryptedBuffer.get(), 0, encryptedBufferSize);

    size_t encryptedDataSize = encryptedBufferSize;

    SECTION("invalid key sizes") {
        SECTION("too small") {
            foeCryptoKey invalidKey = FOE_NULL_HANDLE;
            result = foeCreateCryptoKey(FOE_CRYPTO_XCHACHA20_POLY1305_KEY_SIZE - 1,
                                        invalidKeyData.data(), &invalidKey);
            REQUIRE(result.value == FOE_SUCCESS);

            result = foeCryptoEncrypt_XChaCha20_Poly1305(invalidKey, nonce1.size(), nonce1.data(),
                                                         cOriginalData.size(), cOriginalData.data(),
                                                         &encryptedDataSize, encryptedBuffer.get());
            REQUIRE(result.value == FOE_CRYPTO_ERROR_INVALID_KEY_SIZE);

            foeDestroyCryptoKey(invalidKey);
        }

        SECTION("too large") {
            foeCryptoKey invalidKey = FOE_NULL_HANDLE;
            result = foeCreateCryptoKey(FOE_CRYPTO_XCHACHA20_POLY1305_KEY_SIZE + 1,
                                        invalidKeyData.data(), &invalidKey);
            REQUIRE(result.value == FOE_SUCCESS);

            result = foeCryptoEncrypt_XChaCha20_Poly1305(invalidKey, nonce1.size(), nonce1.data(),
                                                         cOriginalData.size(), cOriginalData.data(),
                                                         &encryptedDataSize, encryptedBuffer.get());
            REQUIRE(result.value == FOE_CRYPTO_ERROR_INVALID_KEY_SIZE);

            foeDestroyCryptoKey(invalidKey);
        }
    }

    SECTION("invalid nonce sizes") {
        foeCryptoKey key = FOE_NULL_HANDLE;

        // Create Valid Key
        result = foeCreateCryptoKey(validKeyData.size(), validKeyData.data(), &key);
        REQUIRE(result.value == FOE_SUCCESS);
        REQUIRE(key != FOE_NULL_HANDLE);

        SECTION("too small") {
            size_t encryptedDataSize = encryptedBufferSize;
            result = foeCryptoEncrypt_XChaCha20_Poly1305(key, nonce1.size() - 1, nonce1.data(),
                                                         cOriginalData.size(), cOriginalData.data(),
                                                         &encryptedDataSize, encryptedBuffer.get());
            REQUIRE(result.value == FOE_CRYPTO_ERROR_INVALID_NONCE_SIZE);
        }

        SECTION("too large") {
            size_t encryptedDataSize = encryptedBufferSize;
            result = foeCryptoEncrypt_XChaCha20_Poly1305(key, nonce1.size() + 1, nonce1.data(),
                                                         cOriginalData.size(), cOriginalData.data(),
                                                         &encryptedDataSize, encryptedBuffer.get());
            REQUIRE(result.value == FOE_CRYPTO_ERROR_INVALID_NONCE_SIZE);
        }

        foeDestroyCryptoKey(key);
    }

    SECTION("destination buffer too small") {
        foeCryptoKey key = FOE_NULL_HANDLE;

        // Create Valid Key
        result = foeCreateCryptoKey(validKeyData.size(), validKeyData.data(), &key);
        REQUIRE(result.value == FOE_SUCCESS);
        REQUIRE(key != FOE_NULL_HANDLE);

        // Attempt encryption
        size_t encryptedBufferSize =
            cOriginalData.size() + FOE_CRYPTO_XCHACHA20_POLY1305_ENCRYPTION_OVERHEAD - 1;
        std::unique_ptr<unsigned char[]> encryptedBuffer(new unsigned char[encryptedBufferSize]);
        memset(encryptedBuffer.get(), 0, encryptedBufferSize);

        size_t encryptedDataSize = encryptedBufferSize;
        result = foeCryptoEncrypt_XChaCha20_Poly1305(key, nonce1.size(), nonce1.data(),
                                                     cOriginalData.size(), cOriginalData.data(),
                                                     &encryptedDataSize, encryptedBuffer.get());
        REQUIRE(result.value == FOE_CRYPTO_ERROR_DESTINATION_BUFFER_TOO_SMALL);

        foeDestroyCryptoKey(key);
    }
}

TEST_CASE("XChaCha20 Poly1305 - decryption failure cases") {
    foeCryptoKey key = FOE_NULL_HANDLE;
    foeResultSet result;

    // Create Valid Key
    result = foeCreateCryptoKey(validKeyData.size(), validKeyData.data(), &key);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(key != FOE_NULL_HANDLE);

    // Create invalid key
    foeCryptoKey invalidKey = FOE_NULL_HANDLE;
    result = foeCreateCryptoKey(FOE_CRYPTO_XCHACHA20_POLY1305_KEY_SIZE, invalidKeyData.data(),
                                &invalidKey);
    REQUIRE(result.value == FOE_SUCCESS);

    // Successful encryption
    size_t encryptedBufferSize =
        cOriginalData.size() + FOE_CRYPTO_XCHACHA20_POLY1305_ENCRYPTION_OVERHEAD;
    std::unique_ptr<unsigned char[]> encryptedBuffer(new unsigned char[encryptedBufferSize]);
    memset(encryptedBuffer.get(), 0, encryptedBufferSize);

    size_t encryptedDataSize = encryptedBufferSize;
    result = foeCryptoEncrypt_XChaCha20_Poly1305(key, nonce1.size(), nonce1.data(),
                                                 cOriginalData.size(), cOriginalData.data(),
                                                 &encryptedDataSize, encryptedBuffer.get());
    REQUIRE(result.value == FOE_SUCCESS);

    REQUIRE(encryptedDataSize == encryptedBufferSize);

    // Decryption
    size_t decryptedDataBufferSize = encryptedBufferSize;
    std::unique_ptr<unsigned char[]> decryptedBuffer(new unsigned char[decryptedDataBufferSize]);

    { // Successful decryption baseline
        size_t decryptedDataSize = decryptedDataBufferSize;
        result = foeCryptoDecrypt_XChaCha20_Poly1305(key, nonce1.size(), nonce1.data(),
                                                     encryptedDataSize, encryptedBuffer.get(),
                                                     &decryptedDataSize, decryptedBuffer.get());
        REQUIRE(result.value == FOE_CRYPTO_SUCCESS);
    }

    SECTION("any byte in encrypted buffer modified") {
        for (size_t i = 0; i < encryptedDataSize; ++i) {
            std::unique_ptr<unsigned char[]> encryptedBufferCopy{
                new unsigned char[encryptedDataSize]};
            memcpy(encryptedBufferCopy.get(), encryptedBuffer.get(), encryptedDataSize);

            ++encryptedBufferCopy[i];

            size_t decryptedDataSize = decryptedDataBufferSize;
            result = foeCryptoDecrypt_XChaCha20_Poly1305(
                key, nonce1.size(), nonce1.data(), encryptedDataSize, encryptedBufferCopy.get(),
                &decryptedDataSize, decryptedBuffer.get());
            REQUIRE(result.value == FOE_CRYPTO_ERROR_FAILED_TO_DECRYPT);
        }
    }
    SECTION("same key/different nonce") {
        size_t decryptedDataSize = decryptedDataBufferSize;
        result = foeCryptoDecrypt_XChaCha20_Poly1305(key, nonce2.size(), nonce2.data(),
                                                     encryptedDataSize, encryptedBuffer.get(),
                                                     &decryptedDataSize, decryptedBuffer.get());
        REQUIRE(result.value == FOE_CRYPTO_ERROR_FAILED_TO_DECRYPT);
    }
    SECTION("different key/same nonce") {
        size_t decryptedDataSize = decryptedDataBufferSize;
        result = foeCryptoDecrypt_XChaCha20_Poly1305(invalidKey, nonce1.size(), nonce1.data(),
                                                     encryptedDataSize, encryptedBuffer.get(),
                                                     &decryptedDataSize, decryptedBuffer.get());
        REQUIRE(result.value == FOE_CRYPTO_ERROR_FAILED_TO_DECRYPT);
    }
    SECTION("different key/nonce") {
        size_t decryptedDataSize = decryptedDataBufferSize;
        result = foeCryptoDecrypt_XChaCha20_Poly1305(invalidKey, nonce2.size(), nonce2.data(),
                                                     encryptedDataSize, encryptedBuffer.get(),
                                                     &decryptedDataSize, decryptedBuffer.get());
        REQUIRE(result.value == FOE_CRYPTO_ERROR_FAILED_TO_DECRYPT);
    }

    SECTION("nonce too small") {
        size_t decryptedDataSize = decryptedDataBufferSize;
        result = foeCryptoDecrypt_XChaCha20_Poly1305(invalidKey, nonce1.size() - 1, nonce1.data(),
                                                     encryptedDataSize, encryptedBuffer.get(),
                                                     &decryptedDataSize, decryptedBuffer.get());
        REQUIRE(result.value == FOE_CRYPTO_ERROR_INVALID_NONCE_SIZE);
    }

    SECTION("nonce too large") {
        size_t decryptedDataSize = decryptedDataBufferSize;
        result = foeCryptoDecrypt_XChaCha20_Poly1305(invalidKey, nonce1.size() - 1, nonce1.data(),
                                                     encryptedDataSize, encryptedBuffer.get(),
                                                     &decryptedDataSize, decryptedBuffer.get());
        REQUIRE(result.value == FOE_CRYPTO_ERROR_INVALID_NONCE_SIZE);
    }

    SECTION("key too small") {
        foeCryptoKey invalidKey = FOE_NULL_HANDLE;
        result = foeCreateCryptoKey(FOE_CRYPTO_XCHACHA20_POLY1305_KEY_SIZE - 1,
                                    invalidKeyData.data(), &invalidKey);
        REQUIRE(result.value == FOE_SUCCESS);

        size_t decryptedDataSize = decryptedDataBufferSize;
        result = foeCryptoDecrypt_XChaCha20_Poly1305(invalidKey, nonce2.size(), nonce2.data(),
                                                     encryptedDataSize, encryptedBuffer.get(),
                                                     &decryptedDataSize, decryptedBuffer.get());
        REQUIRE(result.value == FOE_CRYPTO_ERROR_INVALID_KEY_SIZE);

        foeDestroyCryptoKey(invalidKey);
    }

    SECTION("key too large") {
        foeCryptoKey invalidKey = FOE_NULL_HANDLE;
        result = foeCreateCryptoKey(FOE_CRYPTO_XCHACHA20_POLY1305_KEY_SIZE + 1,
                                    invalidKeyData.data(), &invalidKey);
        REQUIRE(result.value == FOE_SUCCESS);

        size_t decryptedDataSize = decryptedDataBufferSize;
        result = foeCryptoDecrypt_XChaCha20_Poly1305(invalidKey, nonce2.size(), nonce2.data(),
                                                     encryptedDataSize, encryptedBuffer.get(),
                                                     &decryptedDataSize, decryptedBuffer.get());
        REQUIRE(result.value == FOE_CRYPTO_ERROR_INVALID_KEY_SIZE);

        foeDestroyCryptoKey(invalidKey);
    }

    SECTION("destination buffer too small fails") {
        size_t decryptedDataBufferSize = cOriginalData.size() - 1;
        std::unique_ptr<unsigned char[]> decryptedBuffer(
            new unsigned char[decryptedDataBufferSize]);

        size_t decryptedDataSize = decryptedDataBufferSize;
        result = foeCryptoDecrypt_XChaCha20_Poly1305(key, nonce2.size(), nonce2.data(),
                                                     encryptedDataSize, encryptedBuffer.get(),
                                                     &decryptedDataSize, decryptedBuffer.get());
        REQUIRE(result.value == FOE_CRYPTO_ERROR_DESTINATION_BUFFER_TOO_SMALL);
    }

    // Cleanuop
    foeDestroyCryptoKey(invalidKey);
    foeDestroyCryptoKey(key);
}