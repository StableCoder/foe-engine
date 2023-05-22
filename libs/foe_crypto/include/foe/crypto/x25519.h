// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_CRYPTO_X25519_H
#define FOE_CRYPTO_X25519_H

#include <foe/crypto/export.h>
#include <foe/crypto/key.h>
#include <foe/result.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FOE_CRYPTO_X25519_KEY_SIZE 32U

// Generates keys of FOE_CRYPTO_X25519_KEY_SIZE size
FOE_CRYPTO_EXPORT
foeResultSet foeCryptoCreateKeyPairX25519(foeCryptoKey *pPrivateKey, foeCryptoKey *pPublicKey);

// Uses/creates keys of FOE_CRYPTO_X25519_KEY_SIZE size
FOE_CRYPTO_EXPORT
foeResultSet foeCryptoPerformKeyExchangeX25519(foeCryptoKey privateKey,
                                               foeCryptoKey publicKey,
                                               foeCryptoKey hashKey1,
                                               foeCryptoKey hashKey2,
                                               foeCryptoKey *pSharedKey);

#ifdef __cplusplus
}
#endif

#endif // FOE_CRYPTO_X25519_H