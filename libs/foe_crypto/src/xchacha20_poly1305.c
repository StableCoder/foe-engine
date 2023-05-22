// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/crypto/xchacha20_poly1305.h>

#include <sodium/crypto_aead_xchacha20poly1305.h>

#include "result.h"

_Static_assert(FOE_CRYPTO_XCHACHA20_POLY1305_KEY_SIZE ==
                   crypto_aead_xchacha20poly1305_IETF_KEYBYTES,
               "key size is incorrect");

_Static_assert(FOE_CRYPTO_XCHACHA20_POLY1305_NONCE_SIZE ==
                   crypto_aead_xchacha20poly1305_ietf_NPUBBYTES,
               "nonce size is incorrect");

_Static_assert(FOE_CRYPTO_XCHACHA20_POLY1305_ENCRYPTION_OVERHEAD ==
                   crypto_aead_xchacha20poly1305_IETF_ABYTES,
               "overhead size is incorrect");

foeResultSet foeCryptoEncrypt_XChaCha20_Poly1305(foeCryptoKey key,
                                                 size_t nonceSize,
                                                 void const *pNonce,
                                                 size_t dataSize,
                                                 void const *pData,
                                                 size_t *pEncryptedDataSize,
                                                 void *pEncryptedData) {
    if (foeCryptoGetKeySize(key) != FOE_CRYPTO_XCHACHA20_POLY1305_KEY_SIZE)
        return to_foeResult(FOE_CRYPTO_ERROR_INVALID_KEY_SIZE);

    if (nonceSize != FOE_CRYPTO_XCHACHA20_POLY1305_NONCE_SIZE)
        return to_foeResult(FOE_CRYPTO_ERROR_INVALID_NONCE_SIZE);

    // Make sure the destination/encrypted buffer is large enough
    if (dataSize + FOE_CRYPTO_XCHACHA20_POLY1305_ENCRYPTION_OVERHEAD > *pEncryptedDataSize)
        return to_foeResult(FOE_CRYPTO_ERROR_DESTINATION_BUFFER_TOO_SMALL);

    unsigned long long actualEncryptedDataSize;
    if (crypto_aead_xchacha20poly1305_ietf_encrypt(pEncryptedData, &actualEncryptedDataSize, pData,
                                                   dataSize, NULL, 0, NULL, pNonce,
                                                   foeCryptoGetKeyData(key)) != 0) {
        return to_foeResult(FOE_CRYPTO_ERROR_FAILED_TO_ENCRYPT);
    }

    *pEncryptedDataSize = actualEncryptedDataSize;
    return to_foeResult(FOE_CRYPTO_SUCCESS);
}

foeResultSet foeCryptoDecrypt_XChaCha20_Poly1305(foeCryptoKey key,
                                                 size_t nonceSize,
                                                 void const *pNonce,
                                                 size_t encryptedDataSize,
                                                 void const *pEncryptedData,
                                                 size_t *pDecryptedDataSize,
                                                 void *pDecryptedData) {
    if (foeCryptoGetKeySize(key) != FOE_CRYPTO_XCHACHA20_POLY1305_KEY_SIZE)
        return to_foeResult(FOE_CRYPTO_ERROR_INVALID_KEY_SIZE);

    if (nonceSize != FOE_CRYPTO_XCHACHA20_POLY1305_NONCE_SIZE)
        return to_foeResult(FOE_CRYPTO_ERROR_INVALID_NONCE_SIZE);

    // Make sure the destination/decrypted buffer is large enough
    if (encryptedDataSize > *pDecryptedDataSize + FOE_CRYPTO_XCHACHA20_POLY1305_ENCRYPTION_OVERHEAD)
        return to_foeResult(FOE_CRYPTO_ERROR_DESTINATION_BUFFER_TOO_SMALL);

    unsigned long long actualDecryptedDataSize;
    if (crypto_aead_xchacha20poly1305_ietf_decrypt(pDecryptedData, &actualDecryptedDataSize, NULL,
                                                   pEncryptedData, encryptedDataSize, NULL, 0,
                                                   pNonce, foeCryptoGetKeyData(key)) != 0) {
        return to_foeResult(FOE_CRYPTO_ERROR_FAILED_TO_DECRYPT);
    }

    *pDecryptedDataSize = actualDecryptedDataSize;
    return to_foeResult(FOE_CRYPTO_SUCCESS);
}