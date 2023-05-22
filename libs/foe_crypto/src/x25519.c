// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/crypto/x25519.h>

#include <foe/crypto/memory.h>
#include <foe/crypto/random.h>
#include <sodium/crypto_generichash.h>
#include <sodium/crypto_scalarmult_curve25519.h>

#include "result.h"

#include <assert.h>
#include <string.h>

// Verify Key Sizes
_Static_assert(FOE_CRYPTO_X25519_KEY_SIZE == crypto_scalarmult_curve25519_BYTES,
               "ED25519 exchange key size is incorrect");

foeResultSet foeCryptoCreateKeyPairX25519(foeCryptoKey *pPrivateKey, foeCryptoKey *pPublicKey) {
    uint8_t privateKeyData[FOE_CRYPTO_X25519_KEY_SIZE];
    uint8_t publicKeyData[FOE_CRYPTO_X25519_KEY_SIZE];

    // Generate some random secret data (32-bytes for ed25519)
    foeCryptoGenerateRandomData(sizeof(privateKeyData), &privateKeyData);

    // Generate the key pair
    if (crypto_scalarmult_curve25519_base(publicKeyData, privateKeyData) != 0)
        return to_foeResult(FOE_CRYPTO_ERROR_FAILED_TO_CREATE_KEY_PAIR);

    foeCryptoKey privateKey = FOE_NULL_HANDLE;
    foeCryptoKey publicKey = FOE_NULL_HANDLE;

    foeResultSet result = foeCreateCryptoKey(sizeof(privateKeyData), privateKeyData, &privateKey);
    if (result.value != FOE_SUCCESS)
        goto EXCHANGE_KEY_PAIR_CREATE_FAILED;

    assert(memcmp(privateKeyData, foeCryptoGetKeyData(privateKey), sizeof(privateKeyData)) == 0);

    result = foeCreateCryptoKey(sizeof(publicKeyData), publicKeyData, &publicKey);
    if (result.value != FOE_SUCCESS)
        goto EXCHANGE_KEY_PAIR_CREATE_FAILED;

    assert(memcmp(publicKeyData, foeCryptoGetKeyData(publicKey), sizeof(publicKeyData)) == 0);

EXCHANGE_KEY_PAIR_CREATE_FAILED:
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

foeResultSet foeCryptoPerformKeyExchangeX25519(foeCryptoKey privateKey,
                                               foeCryptoKey publicKey,
                                               foeCryptoKey hashKey1,
                                               foeCryptoKey hashKey2,
                                               foeCryptoKey *pSharedKey) {
    if (foeCryptoGetKeySize(privateKey) != FOE_CRYPTO_X25519_KEY_SIZE ||
        foeCryptoGetKeySize(publicKey) != FOE_CRYPTO_X25519_KEY_SIZE ||
        foeCryptoGetKeySize(hashKey1) != FOE_CRYPTO_X25519_KEY_SIZE ||
        foeCryptoGetKeySize(hashKey2) != FOE_CRYPTO_X25519_KEY_SIZE)
        return to_foeResult(FOE_CRYPTO_ERROR_INVALID_KEY_SIZE);

    uint8_t sharedKeyData[FOE_CRYPTO_X25519_KEY_SIZE];
    foeCryptoKey sharedKey = FOE_NULL_HANDLE;
    foeResultSet result;

    if (crypto_scalarmult_curve25519(sharedKeyData, foeCryptoGetKeyData(privateKey),
                                     foeCryptoGetKeyData(publicKey)) != 0) {
        result = to_foeResult(FOE_CRYPTO_ERROR_FAILED_TO_PERFORM_KEY_EXCHANGE);
        goto KEY_EXCHANGE_FAILED;
    }

    // To mitigate subtle attacks due to the fact many (private, public) pairs produce the same
    // result, using the output of the multiplication q directly as a shared key is not recommended.
    //
    // A better way to compute a shared key is hash(shared_key ‖ public_key_1 ‖ public_key_2), with
    // public_key_1 and public_key_2 being the public keys.
    crypto_generichash_state hashState;
    if (crypto_generichash_init(&hashState, NULL, 0U, FOE_CRYPTO_X25519_KEY_SIZE) != 0 ||
        crypto_generichash_update(&hashState, sharedKeyData, FOE_CRYPTO_X25519_KEY_SIZE) != 0 ||
        crypto_generichash_update(&hashState, foeCryptoGetKeyData(hashKey1),
                                  FOE_CRYPTO_X25519_KEY_SIZE) != 0 ||
        crypto_generichash_update(&hashState, foeCryptoGetKeyData(hashKey2),
                                  FOE_CRYPTO_X25519_KEY_SIZE) != 0 ||
        crypto_generichash_final(&hashState, sharedKeyData, FOE_CRYPTO_X25519_KEY_SIZE) != 0) {
        result = to_foeResult(FOE_CRYPTO_ERROR_FAILED_TO_PERFORM_KEY_EXCHANGE);
        goto KEY_EXCHANGE_FAILED;
    }

    result = foeCreateCryptoKey(sizeof(sharedKeyData), sharedKeyData, &sharedKey);
    if (result.value != FOE_SUCCESS)
        goto KEY_EXCHANGE_FAILED;

KEY_EXCHANGE_FAILED:
    foeCryptoZeroMemory(sizeof(sharedKeyData), sharedKeyData);

    if (result.value == FOE_SUCCESS) {
        *pSharedKey = sharedKey;
    } else {
        if (sharedKey != FOE_NULL_HANDLE)
            foeDestroyCryptoKey(sharedKey);
    }

    foeCryptoZeroMemory(sizeof(foeCryptoKey), &sharedKey);

    return to_foeResult(FOE_CRYPTO_SUCCESS);
}