// Copyright (C) 2023-2024 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/crypto/ed25519.h>

#include <foe/crypto/memory.h>
#include <foe/crypto/random.h>
#include <sodium/crypto_sign_ed25519.h>

#include "result.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

// Verify Key Sizes
_Static_assert(FOE_CRYPTO_ED25519_PRIVATE_KEY_SIZE == crypto_sign_ed25519_SECRETKEYBYTES,
               "ED25519 private signing key size is incorrect");
_Static_assert(FOE_CRYPTO_ED25519_PUBLIC_KEY_SIZE == crypto_sign_ed25519_PUBLICKEYBYTES,
               "ED25519 public signing key size is incorrect");

foeResultSet foeCryptoCreateKeyPairED25519(foeCryptoKey *pPrivateKey, foeCryptoKey *pPublicKey) {
    uint8_t privateKeyData[FOE_CRYPTO_ED25519_PRIVATE_KEY_SIZE];
    uint8_t publicKeyData[FOE_CRYPTO_ED25519_PUBLIC_KEY_SIZE];

    if (crypto_sign_ed25519_keypair(publicKeyData, privateKeyData) != 0)
        return to_foeResult(FOE_CRYPTO_ERROR_FAILED_TO_CREATE_KEY_PAIR);

    foeCryptoKey privateKey = FOE_NULL_HANDLE;
    foeCryptoKey publicKey = FOE_NULL_HANDLE;

    foeResultSet result = foeCreateCryptoKey(sizeof(privateKeyData), privateKeyData, &privateKey);
    if (result.value != FOE_SUCCESS)
        goto CREATE_KEY_PAIR_FAILED;

    assert(memcmp(privateKeyData, foeCryptoGetKeyData(privateKey), sizeof(privateKeyData)) == 0);

    result = foeCreateCryptoKey(sizeof(publicKeyData), publicKeyData, &publicKey);
    if (result.value != FOE_SUCCESS)
        goto CREATE_KEY_PAIR_FAILED;

    assert(memcmp(publicKeyData, foeCryptoGetKeyData(publicKey), sizeof(publicKeyData)) == 0);

CREATE_KEY_PAIR_FAILED:
    // Zero the local key memory to help prevent key data leaks
    foeCryptoZeroMemory(sizeof(privateKeyData), privateKeyData);
    foeCryptoZeroMemory(sizeof(publicKeyData), publicKeyData);

    if (result.value == FOE_SUCCESS) {
        *pPrivateKey = privateKey;
        *pPublicKey = publicKey;
    } else {
        if (privateKey != FOE_NULL_HANDLE)
            foeDestroyCryptoKey(privateKey);

        if (publicKey != FOE_NULL_HANDLE)
            foeDestroyCryptoKey(publicKey);
    }

    foeCryptoZeroMemory(sizeof(foeCryptoKey), &privateKey);
    foeCryptoZeroMemory(sizeof(foeCryptoKey), &publicKey);

    return result;
}

foeResultSet foeCryptoCreatePublicKeyED25519(foeCryptoKey privateKey, foeCryptoKey *pPublicKey) {
    if (foeCryptoGetKeySize(privateKey) != FOE_CRYPTO_ED25519_PRIVATE_KEY_SIZE)
        return to_foeResult(FOE_CRYPTO_ERROR_INVALID_KEY_SIZE);

    uint8_t publicKeyData[FOE_CRYPTO_ED25519_PUBLIC_KEY_SIZE];

    if (crypto_sign_ed25519_sk_to_pk(publicKeyData, foeCryptoGetKeyData(privateKey)) != 0)
        return to_foeResult(FOE_CRYPTO_ERROR_FAILED_TO_CREATE_KEY_PAIR);

    foeCryptoKey publicKey = FOE_NULL_HANDLE;

    foeResultSet result = foeCreateCryptoKey(sizeof(publicKeyData), publicKeyData, &publicKey);
    if (result.value != FOE_SUCCESS)
        goto GENERATE_PUBLIC_KEY_FAILED;

GENERATE_PUBLIC_KEY_FAILED:
    foeCryptoZeroMemory(sizeof(publicKeyData), publicKeyData);

    if (result.value == FOE_SUCCESS) {
        *pPublicKey = publicKey;
    } else {
        if (publicKey != FOE_NULL_HANDLE)
            foeDestroyCryptoKey(publicKey);
    }

    foeCryptoZeroMemory(sizeof(foeCryptoKey), &publicKey);

    return result;
}

foeResultSet foeCryptoSignDataED25519(foeCryptoKey privateKey,
                                      size_t dataSize,
                                      void const *pData,
                                      void *pSignature) {
    if (foeCryptoGetKeySize(privateKey) != FOE_CRYPTO_ED25519_PRIVATE_KEY_SIZE)
        return to_foeResult(FOE_CRYPTO_ERROR_INVALID_KEY_SIZE);

    unsigned long long actualSignatureSize = FOE_CRYPTO_ED25519_SIGNATURE_SIZE;
    if (crypto_sign_ed25519_detached(pSignature, &actualSignatureSize, pData, dataSize,
                                     foeCryptoGetKeyData(privateKey)) != 0)
        return to_foeResult(FOE_CRYPTO_ERROR_FAILED_TO_SIGN);

    assert(actualSignatureSize == FOE_CRYPTO_ED25519_SIGNATURE_SIZE);

    return to_foeResult(FOE_CRYPTO_SUCCESS);
}

foeResultSet foeCryptoVerifyDataED25519(foeCryptoKey publicKey,
                                        size_t dataSize,
                                        void const *pData,
                                        void const *pSignature) {
    if (foeCryptoGetKeySize(publicKey) != FOE_CRYPTO_ED25519_PUBLIC_KEY_SIZE)
        return to_foeResult(FOE_CRYPTO_ERROR_INVALID_KEY_SIZE);

    if (crypto_sign_ed25519_verify_detached(pSignature, pData, dataSize,
                                            foeCryptoGetKeyData(publicKey)) != 0)
        return to_foeResult(FOE_CRYPTO_ERROR_FAILED_TO_VERIFY);

    return to_foeResult(FOE_CRYPTO_SUCCESS);
}
