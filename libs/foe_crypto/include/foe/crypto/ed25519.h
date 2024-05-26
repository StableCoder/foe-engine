// Copyright (C) 2023-2024 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_CRYPTO_ED25519_H
#define FOE_CRYPTO_ED25519_H

#include <foe/crypto/export.h>
#include <foe/crypto/key.h>
#include <foe/result.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FOE_CRYPTO_ED25519_PUBLIC_KEY_SIZE  32
#define FOE_CRYPTO_ED25519_PRIVATE_KEY_SIZE 64

#define FOE_CRYPTO_ED25519_SIGNATURE_SIZE 64

// Creates private key of FOE_CRYPTO_ED25519_SIGNATURE_PRIVATE_KEY_SIZE size
// Creates public key of FOE_CRYPTO_ED25519_SIGNATURE_PUBLIC_KEY_SIZE size
FOE_CRYPTO_EXPORT
foeResultSet foeCryptoCreateKeyPairED25519(foeCryptoKey *pPrivateKey, foeCryptoKey *pPublicKey);

// Uses key of FOE_CRYPTO_ED25519_SIGNATURE_PRIVATE_KEY_SIZE size
// Creates public key of FOE_CRYPTO_ED25519_SIGNATURE_PUBLIC_KEY_SIZE size
FOE_CRYPTO_EXPORT
foeResultSet foeCryptoCreatePublicKeyED25519(foeCryptoKey privateKey, foeCryptoKey *pPublicKey);

// Uses key of FOE_CRYPTO_ED25519_SIGNATURE_PRIVATE_KEY_SIZE size
// signature size will be FOE_CRYPTO_ED25519_SIGNATURE_SIZE bytes
FOE_CRYPTO_EXPORT
foeResultSet foeCryptoSignDataED25519(foeCryptoKey privateKey,
                                      size_t dataSize,
                                      void const *pData,
                                      void *pSignature);

// Uses key of FOE_CRYPTO_ED25519_SIGNATURE_PUBLIC_KEY_SIZE size
// signature size must be FOE_CRYPTO_ED25519_SIGNATURE_SIZE bytes
FOE_CRYPTO_EXPORT
foeResultSet foeCryptoVerifyDataED25519(foeCryptoKey publicKey,
                                        size_t dataSize,
                                        void const *pData,
                                        void const *pSignature);

#ifdef __cplusplus
}
#endif

#endif // FOE_CRYPTO_ED25519_H