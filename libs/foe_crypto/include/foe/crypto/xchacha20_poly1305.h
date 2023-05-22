// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_CRYPTO_XCHACHA20_POLY1305_H
#define FOE_CRYPTO_XCHACHA20_POLY1305_H

#include <foe/crypto/export.h>
#include <foe/crypto/key.h>
#include <foe/result.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FOE_CRYPTO_XCHACHA20_POLY1305_KEY_SIZE            32U
#define FOE_CRYPTO_XCHACHA20_POLY1305_NONCE_SIZE          24U
#define FOE_CRYPTO_XCHACHA20_POLY1305_ENCRYPTION_OVERHEAD 16U

// Key must be FOE_CRYPTO_XCHACHA20_POLY1305_KEY_SIZE bytes
// Nonce must be FOE_CRYPTO_XCHACHA20_POLY1305_NONCE_SIZE bytes
// Destination must be at least (dataSize + FOE_CRYPTO_XCHACHA20_POLY1305_ENCRYPTION_OVERHEAD) bytes
FOE_CRYPTO_EXPORT
foeResultSet foeCryptoEncrypt_XChaCha20_Poly1305(foeCryptoKey key,
                                                 size_t nonceSize,
                                                 void const *pNonce,
                                                 size_t dataSize,
                                                 void const *pData,
                                                 size_t *pEncryptedDataSize,
                                                 void *pEncryptedData);

// Key must be FOE_CRYPTO_XCHACHA20_POLY1305_KEY_SIZE bytes
// Nonce must be FOE_CRYPTO_XCHACHA20_POLY1305_NONCE_SIZE bytes
// Destination must be at least (encryptedDataSize -
// FOE_CRYPTO_XCHACHA20_POLY1305_ENCRYPTION_OVERHEAD) bytes
FOE_CRYPTO_EXPORT
foeResultSet foeCryptoDecrypt_XChaCha20_Poly1305(foeCryptoKey key,
                                                 size_t nonceSize,
                                                 void const *pNonce,
                                                 size_t encryptedDataSize,
                                                 void const *pEncryptedData,
                                                 size_t *pDecryptedDataSize,
                                                 void *pDecryptedData);

#ifdef __cplusplus
}
#endif

#endif // FOE_CRYPTO_XCHACHA20_POLY1305_H