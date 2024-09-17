// Copyright (C) 2023-2024 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/crypto/aes_256_gcm.h>
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

constexpr std::string_view validKeyData = "abcdefghijklmnopqrstuvwxyzabcdef";
static_assert(validKeyData.size() == FOE_CRYPTO_AES_256_GCM_KEY_SIZE);

constexpr std::string_view invalidKeyData =
    "12345678901234567890123456789012345678901234567890123456789012345";

constexpr std::string_view nonce1 = "123456789012";
static_assert(nonce1.size() == FOE_CRYPTO_AES_256_GCM_NONCE_SIZE);

constexpr std::string_view nonce2 = "210987654321";
static_assert(nonce2.size() == FOE_CRYPTO_AES_256_GCM_NONCE_SIZE);

static_assert(sizeof(nonce1) == sizeof(nonce2));
} // namespace

// Only x86-64 or AppleSilicon are known to have AES acceleration support in libsodium
#if (defined(__APPLE__) || defined(__x86_64__) || defined(_WIN64)) && !defined(__MINGW32__)

TEST_CASE("AES 256 GCM - Detect HW acceleration is available") {
    REQUIRE(foeCrypto_AES_256_GCM_isHardwareAccelerated());
}

TEST_CASE("AES 256 GCM - encryption success cases") {
    foeCryptoKey key = FOE_NULL_HANDLE;
    foeCryptoContext_AES_256_GCM context = FOE_NULL_HANDLE;
    foeResultSet result;

    // Create key
    result = foeCreateCryptoKey(validKeyData.size(), validKeyData.data(), &key);
    REQUIRE(result.value == FOE_SUCCESS);

    // Create AES GCM context
    result = foeCreateContext_AES_256_GCM(key, &context);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(context != FOE_NULL_HANDLE);

    // Don't need the key after context creation
    foeDestroyCryptoKey(key);

    // Encrypt Data
    constexpr size_t encryptedBufferSize =
        cOriginalData.size() + FOE_CRYPTO_AES_256_GCM_ENCRYPTION_OVERHEAD;
    std::unique_ptr<unsigned char[]> encryptedBuffer(new unsigned char[encryptedBufferSize + 16]);
    memset(encryptedBuffer.get(), 0, encryptedBufferSize + 16);

    size_t encryptedDataSize = encryptedBufferSize;
    SECTION("destination buffer size is exact required") {
        result = foeCryptoEncrypt_AES_256_GCM(context, nonce1.size(), nonce1.data(),
                                              cOriginalData.size(), cOriginalData.data(),
                                              &encryptedDataSize, encryptedBuffer.get());
        REQUIRE(result.value == FOE_SUCCESS);
        REQUIRE(encryptedDataSize == encryptedBufferSize);
    }
    SECTION("destination buffer size is too large") {
        encryptedDataSize += 16;
        result = foeCryptoEncrypt_AES_256_GCM(context, nonce1.size(), nonce1.data(),
                                              cOriginalData.size(), cOriginalData.data(),
                                              &encryptedDataSize, encryptedBuffer.get());
        REQUIRE(result.value == FOE_SUCCESS);
        REQUIRE(encryptedDataSize == encryptedBufferSize);
    }

    // Decrypt Data
    constexpr size_t decryptedDataBufferSize = cOriginalData.size();
    std::unique_ptr<unsigned char[]> decryptedBuffer(new unsigned char[decryptedDataBufferSize]);

    size_t decryptedDataSize = decryptedDataBufferSize;
    result = foeCryptoDecrypt_AES_256_GCM(context, nonce1.size(), nonce1.data(), encryptedDataSize,
                                          encryptedBuffer.get(), &decryptedDataSize,
                                          decryptedBuffer.get());
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(decryptedDataSize == decryptedDataBufferSize);
    REQUIRE(memcmp(cOriginalData.data(), decryptedBuffer.get(), cOriginalData.size()) == 0);

    foeDestroyContext_AES_256_GCM(context);
}

TEST_CASE("AES 256 GCM - decryption success cases") {
    foeCryptoKey key = FOE_NULL_HANDLE;
    foeCryptoContext_AES_256_GCM context = FOE_NULL_HANDLE;
    foeResultSet result;

    // Create key
    result = foeCreateCryptoKey(validKeyData.size(), validKeyData.data(), &key);
    REQUIRE(result.value == FOE_SUCCESS);

    // Create AES GCM context
    result = foeCreateContext_AES_256_GCM(key, &context);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(context != FOE_NULL_HANDLE);

    // Don't need the key after context creation
    foeDestroyCryptoKey(key);

    // Encrypt Data
    constexpr size_t encryptedBufferSize =
        cOriginalData.size() + FOE_CRYPTO_AES_256_GCM_ENCRYPTION_OVERHEAD;
    std::unique_ptr<unsigned char[]> encryptedBuffer(new unsigned char[encryptedBufferSize + 16]);
    memset(encryptedBuffer.get(), 0, encryptedBufferSize + 16);

    size_t encryptedDataSize = encryptedBufferSize;
    result = foeCryptoEncrypt_AES_256_GCM(context, nonce1.size(), nonce1.data(),
                                          cOriginalData.size(), cOriginalData.data(),
                                          &encryptedDataSize, encryptedBuffer.get());
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(encryptedDataSize == encryptedBufferSize);

    // Decrypt Data
    constexpr size_t decryptedDataBufferSize = cOriginalData.size();
    std::unique_ptr<unsigned char[]> decryptedBuffer(
        new unsigned char[decryptedDataBufferSize + 16]);

    size_t decryptedDataSize = decryptedDataBufferSize;
    SECTION("destination buffer size is exact required") {
        result = foeCryptoDecrypt_AES_256_GCM(context, nonce1.size(), nonce1.data(),
                                              encryptedDataSize, encryptedBuffer.get(),
                                              &decryptedDataSize, decryptedBuffer.get());
        REQUIRE(result.value == FOE_SUCCESS);
        REQUIRE(decryptedDataSize == decryptedDataBufferSize);
        REQUIRE(memcmp(cOriginalData.data(), decryptedBuffer.get(), cOriginalData.size()) == 0);
    }
    SECTION("destination buffer size is too large") {
        decryptedDataSize += 16;
        result = foeCryptoDecrypt_AES_256_GCM(context, nonce1.size(), nonce1.data(),
                                              encryptedDataSize, encryptedBuffer.get(),
                                              &decryptedDataSize, decryptedBuffer.get());
        REQUIRE(result.value == FOE_SUCCESS);
        REQUIRE(decryptedDataSize == decryptedDataBufferSize);
        REQUIRE(memcmp(cOriginalData.data(), decryptedBuffer.get(), cOriginalData.size()) == 0);
    }

    foeDestroyContext_AES_256_GCM(context);
}

TEST_CASE("AES 256 GCM - encryption context creation failure cases") {
    foeCryptoKey key = FOE_NULL_HANDLE;
    foeCryptoContext_AES_256_GCM context = FOE_NULL_HANDLE;
    foeResultSet result;

    SECTION("key too small") {
        result =
            foeCreateCryptoKey(FOE_CRYPTO_AES_256_GCM_KEY_SIZE - 1, invalidKeyData.data(), &key);
        REQUIRE(result.value == FOE_SUCCESS);

        result = foeCreateContext_AES_256_GCM(key, &context);
        REQUIRE(result.value == FOE_CRYPTO_ERROR_INVALID_KEY_SIZE);
        REQUIRE(context == FOE_NULL_HANDLE);
    }
    SECTION("key too large") {
        result =
            foeCreateCryptoKey(FOE_CRYPTO_AES_256_GCM_KEY_SIZE + 1, invalidKeyData.data(), &key);
        REQUIRE(result.value == FOE_SUCCESS);

        result = foeCreateContext_AES_256_GCM(key, &context);
        REQUIRE(result.value == FOE_CRYPTO_ERROR_INVALID_KEY_SIZE);
        REQUIRE(context == FOE_NULL_HANDLE);
    }

    foeDestroyCryptoKey(key);
}

TEST_CASE("AES 256 GCM - encryption failure cases") {
    foeCryptoKey key = FOE_NULL_HANDLE;
    foeCryptoContext_AES_256_GCM context = FOE_NULL_HANDLE;
    foeResultSet result;

    // Create valid key
    result = foeCreateCryptoKey(validKeyData.size(), validKeyData.data(), &key);
    REQUIRE(result.value == FOE_SUCCESS);

    // Create AES GCM context
    result = foeCreateContext_AES_256_GCM(key, &context);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(context != FOE_NULL_HANDLE);

    // Don't need the key after context creation
    foeDestroyCryptoKey(key);

    SECTION("invalid nonce sizes") {
        size_t encryptedBufferSize =
            cOriginalData.size() + FOE_CRYPTO_AES_256_GCM_ENCRYPTION_OVERHEAD;
        std::unique_ptr<unsigned char[]> encryptedBuffer(new unsigned char[encryptedBufferSize]);
        memset(encryptedBuffer.get(), 0, encryptedBufferSize);

        SECTION("too small") {
            size_t encryptedDataSize = encryptedBufferSize;
            result = foeCryptoEncrypt_AES_256_GCM(context, nonce1.size() - 1, nonce1.data(),
                                                  cOriginalData.size(), cOriginalData.data(),
                                                  &encryptedDataSize, encryptedBuffer.get());
            REQUIRE(result.value == FOE_CRYPTO_ERROR_INVALID_NONCE_SIZE);
        }

        SECTION("too large") {
            size_t encryptedDataSize = encryptedBufferSize;
            result = foeCryptoEncrypt_AES_256_GCM(context, nonce1.size() + 1, nonce1.data(),
                                                  cOriginalData.size(), cOriginalData.data(),
                                                  &encryptedDataSize, encryptedBuffer.get());
            REQUIRE(result.value == FOE_CRYPTO_ERROR_INVALID_NONCE_SIZE);
        }
    }

    SECTION("destination buffer too small") {
        size_t encryptedBufferSize =
            cOriginalData.size() + FOE_CRYPTO_AES_256_GCM_ENCRYPTION_OVERHEAD - 1;
        std::unique_ptr<unsigned char[]> encryptedBuffer(new unsigned char[encryptedBufferSize]);
        memset(encryptedBuffer.get(), 0, encryptedBufferSize);

        size_t encryptedDataSize = encryptedBufferSize;
        result = foeCryptoEncrypt_AES_256_GCM(context, nonce1.size(), nonce1.data(),
                                              cOriginalData.size(), cOriginalData.data(),
                                              &encryptedDataSize, encryptedBuffer.get());
        REQUIRE(result.value == FOE_CRYPTO_ERROR_DESTINATION_BUFFER_TOO_SMALL);
    }

    foeDestroyContext_AES_256_GCM(context);
}

TEST_CASE("AES 256 GCM - decryption failure cases") {
    foeCryptoKey key = FOE_NULL_HANDLE;
    foeCryptoContext_AES_256_GCM context = FOE_NULL_HANDLE;
    foeResultSet result;

    // Create Keys
    result = foeCreateCryptoKey(validKeyData.size(), validKeyData.data(), &key);
    REQUIRE(result.value == FOE_SUCCESS);

    // Create AES GCM context
    result = foeCreateContext_AES_256_GCM(key, &context);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(context != FOE_NULL_HANDLE);

    // Don't need the key after context creation
    foeDestroyCryptoKey(key);

    // Encrypt Data
    size_t encryptedBufferSize = cOriginalData.size() + FOE_CRYPTO_AES_256_GCM_ENCRYPTION_OVERHEAD;
    std::unique_ptr<unsigned char[]> encryptedBuffer(new unsigned char[encryptedBufferSize]);
    memset(encryptedBuffer.get(), 0, encryptedBufferSize);

    size_t encryptedDataSize = encryptedBufferSize;
    result = foeCryptoEncrypt_AES_256_GCM(context, nonce1.size(), nonce1.data(),
                                          cOriginalData.size(), cOriginalData.data(),
                                          &encryptedDataSize, encryptedBuffer.get());
    REQUIRE(result.value == FOE_SUCCESS);

    REQUIRE(encryptedDataSize == encryptedBufferSize);

    constexpr size_t decryptedDataBufferSize = cOriginalData.size();
    std::unique_ptr<unsigned char[]> decryptedBuffer(new unsigned char[decryptedDataBufferSize]);

    { // Successful decryption baseline
        size_t decryptedDataSize = decryptedDataBufferSize;
        result = foeCryptoDecrypt_AES_256_GCM(context, nonce1.size(), nonce1.data(),
                                              encryptedDataSize, encryptedBuffer.get(),
                                              &decryptedDataSize, decryptedBuffer.get());
        REQUIRE(result.value == FOE_SUCCESS);
    }

    SECTION("any byte in encrypted buffer is modified") {
        for (size_t i = 0; i < encryptedDataSize; ++i) {
            std::unique_ptr<unsigned char[]> encryptedBufferCopy{
                new unsigned char[encryptedDataSize]};
            memcpy(encryptedBufferCopy.get(), encryptedBuffer.get(), encryptedDataSize);

            ++encryptedBufferCopy[i];

            size_t decryptedDataSize = decryptedDataBufferSize;
            result = foeCryptoDecrypt_AES_256_GCM(context, nonce1.size(), nonce1.data(),
                                                  encryptedDataSize, encryptedBufferCopy.get(),
                                                  &decryptedDataSize, decryptedBuffer.get());
            REQUIRE(result.value == FOE_CRYPTO_ERROR_FAILED_TO_DECRYPT);
        }
    }
    SECTION("different nonce") {
        size_t decryptedDataSize = decryptedDataBufferSize;
        result = foeCryptoDecrypt_AES_256_GCM(context, nonce2.size(), nonce2.data(),
                                              encryptedDataSize, encryptedBuffer.get(),
                                              &decryptedDataSize, decryptedBuffer.get());
        REQUIRE(result.value == FOE_CRYPTO_ERROR_FAILED_TO_DECRYPT);
    }
    SECTION("nonce too small") {
        size_t decryptedDataSize = decryptedDataBufferSize;
        result = foeCryptoDecrypt_AES_256_GCM(context, nonce1.size() - 1, nonce1.data(),
                                              encryptedDataSize, encryptedBuffer.get(),
                                              &decryptedDataSize, decryptedBuffer.get());
        REQUIRE(result.value == FOE_CRYPTO_ERROR_INVALID_NONCE_SIZE);
    }

    SECTION("nonce too large") {
        size_t decryptedDataSize = decryptedDataBufferSize;
        result = foeCryptoDecrypt_AES_256_GCM(context, nonce1.size() + 1, nonce1.data(),
                                              encryptedDataSize, encryptedBuffer.get(),
                                              &decryptedDataSize, decryptedBuffer.get());
        REQUIRE(result.value == FOE_CRYPTO_ERROR_INVALID_NONCE_SIZE);
    }

    SECTION("destination buffer too small") {
        size_t decryptedDataSize = decryptedDataBufferSize - 1;
        result = foeCryptoDecrypt_AES_256_GCM(context, nonce2.size(), nonce2.data(),
                                              encryptedDataSize, encryptedBuffer.get(),
                                              &decryptedDataSize, decryptedBuffer.get());
        REQUIRE(result.value == FOE_CRYPTO_ERROR_DESTINATION_BUFFER_TOO_SMALL);
    }

    foeDestroyContext_AES_256_GCM(context);
}

#else

TEST_CASE("AES 256 GCM - Detect HW acceleration is not available") {
    REQUIRE_FALSE(foeCrypto_AES_256_GCM_isHardwareAccelerated());
}

TEST_CASE("AES 256 GCM - Failure (No AES hardware acceleration)") {
    foeCryptoKey key = FOE_NULL_HANDLE;
    foeCryptoContext_AES_256_GCM context = FOE_NULL_HANDLE;
    foeResultSet result;

    // Create Key
    result = foeCreateCryptoKey(validKeyData.size(), validKeyData.data(), &key);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(key != FOE_NULL_HANDLE);

    // Attempt to create AES GCM context
    result = foeCreateContext_AES_256_GCM(key, &context);
    REQUIRE(result.value == FOE_CRYPTO_ERROR_NO_AES_HARDWARE_ACCELERATION_AVAILABLE);
    REQUIRE(context == FOE_NULL_HANDLE);

    foeDestroyCryptoKey(key);
}

#endif