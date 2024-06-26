// Copyright (C) 2023-2024 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_CRYPTO_AES_256_GCM_H
#define FOE_CRYPTO_AES_256_GCM_H

#include <foe/crypto/export.h>
#include <foe/crypto/key.h>
#include <foe/handle.h>
#include <foe/result.h>

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FOE_CRYPTO_AES_256_GCM_KEY_SIZE            32
#define FOE_CRYPTO_AES_256_GCM_NONCE_SIZE          12
#define FOE_CRYPTO_AES_256_GCM_ENCRYPTION_OVERHEAD 16

FOE_DEFINE_HANDLE(foeCryptoContext_AES_256_GCM)

FOE_CRYPTO_EXPORT
bool foeCrypto_AES_256_GCM_isHardwareAccelerated();

// Key must be FOE_CRYPTO_AES_256_GCM_KEY_SIZE bytes
FOE_CRYPTO_EXPORT
foeResultSet foeCreateContext_AES_256_GCM(foeCryptoKey key, foeCryptoContext_AES_256_GCM *pContext);

FOE_CRYPTO_EXPORT
void foeDestroyContext_AES_256_GCM(foeCryptoContext_AES_256_GCM context);

// Nonce must be FOE_CRYPTO_AES_256_GCM_NONCE_SIZE bytes
// Destination must be at least (dataSize + FOE_CRYPTO_AES_256_GCM_ENCRYPTION_OVERHEAD) bytes
FOE_CRYPTO_EXPORT
foeResultSet foeCryptoEncrypt_AES_256_GCM(foeCryptoContext_AES_256_GCM context,
                                          size_t nonceSize,
                                          void const *pNonce,
                                          size_t dataSize,
                                          void const *pData,
                                          size_t *pEncryptedDataSize,
                                          void *pEncryptedData);

// Nonce must be FOE_CRYPTO_AES_256_GCM_NONCE_SIZE bytes
// Destination must be at least (encryptedDataSize - FOE_CRYPTO_AES_256_GCM_ENCRYPTION_OVERHEAD)
// bytes
FOE_CRYPTO_EXPORT
foeResultSet foeCryptoDecrypt_AES_256_GCM(foeCryptoContext_AES_256_GCM context,
                                          size_t nonceSize,
                                          void const *pNonce,
                                          size_t encryptedDataSize,
                                          void const *pEncryptedData,
                                          size_t *pDecryptedDataSize,
                                          void *pDecryptedData);

#ifdef __cplusplus
}
#endif

#endif // FOE_CRYPTO_AES_256_GCM_H