// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/crypto/aes_256_gcm.h>

#include <sodium/core.h>
#include <sodium/crypto_aead_aes256gcm.h>
#include <sodium/utils.h>

#include "result.h"

_Static_assert(FOE_CRYPTO_AES_256_GCM_KEY_SIZE == crypto_aead_aes256gcm_KEYBYTES,
               "key size is incorrect");

_Static_assert(FOE_CRYPTO_AES_256_GCM_NONCE_SIZE == crypto_aead_aes256gcm_NPUBBYTES,
               "nonce size is incorrect");

_Static_assert(FOE_CRYPTO_AES_256_GCM_ENCRYPTION_OVERHEAD == crypto_aead_aes256gcm_ABYTES,
               "overhead size is incorrect");

FOE_DEFINE_HANDLE_CASTS(encryption_state, crypto_aead_aes256gcm_state, foeCryptoContext_AES_256_GCM)

bool foeCrypto_isHardwareAccelerated_AES_256_GCM() {
    if (sodium_init() < 0)
        return false;

    return crypto_aead_aes256gcm_is_available() == 1;
}

foeResultSet foeCreateContext_AES_256_GCM(foeCryptoKey key,
                                          foeCryptoContext_AES_256_GCM *pContext) {
    if (sodium_init() < 0)
        return to_foeResult(FOE_CRYPTO_ERROR_LIBRARY_FAILED_TO_INITIALIZE);

    if (crypto_aead_aes256gcm_is_available() != 1)
        return to_foeResult(FOE_CRYPTO_ERROR_NO_AES_HARDWARE_ACCELERATION_AVAILABLE);

    if (foeCryptoGetKeySize(key) != FOE_CRYPTO_AES_256_GCM_KEY_SIZE)
        return to_foeResult(FOE_CRYPTO_ERROR_INVALID_KEY_SIZE);

    crypto_aead_aes256gcm_state *pNewEncryptionContext =
        sodium_malloc(sizeof(crypto_aead_aes256gcm_state));
    if (pNewEncryptionContext == NULL)
        return to_foeResult(FOE_CRYPTO_ERROR_OUT_OF_MEMORY);

    if (crypto_aead_aes256gcm_beforenm(pNewEncryptionContext, foeCryptoGetKeyData(key)) != 0) {
        sodium_free(pNewEncryptionContext);
        return to_foeResult(FOE_CRYPTO_ERROR_INITIALIZATION_FAILED);
    }

    *pContext = encryption_state_to_handle(pNewEncryptionContext);
    return to_foeResult(FOE_CRYPTO_SUCCESS);
}

void foeDestroyContext_AES_256_GCM(foeCryptoContext_AES_256_GCM context) {
    sodium_free(encryption_state_from_handle(context));
}

foeResultSet foeCryptoEncrypt_AES_256_GCM(foeCryptoContext_AES_256_GCM context,
                                          size_t nonceSize,
                                          void const *pNonce,
                                          size_t dataSize,
                                          void const *pData,
                                          size_t *pEncryptedDataSize,
                                          void *pEncryptedData) {
    if (nonceSize != FOE_CRYPTO_AES_256_GCM_NONCE_SIZE)
        return to_foeResult(FOE_CRYPTO_ERROR_INVALID_NONCE_SIZE);

    // Make sure the destination/encrypted buffer is large enough
    if (dataSize + FOE_CRYPTO_AES_256_GCM_ENCRYPTION_OVERHEAD > *pEncryptedDataSize)
        return to_foeResult(FOE_CRYPTO_ERROR_DESTINATION_BUFFER_TOO_SMALL);

    unsigned long long actualEncryptedDataSize;
    if (crypto_aead_aes256gcm_encrypt_afternm(pEncryptedData, &actualEncryptedDataSize, pData,
                                              dataSize, NULL, 0, NULL, pNonce,
                                              encryption_state_from_handle(context)) != 0) {
        return to_foeResult(FOE_CRYPTO_ERROR_FAILED_TO_ENCRYPT);
    }

    *pEncryptedDataSize = actualEncryptedDataSize;
    return to_foeResult(FOE_CRYPTO_SUCCESS);
}

foeResultSet foeCryptoDecrypt_AES_256_GCM(foeCryptoContext_AES_256_GCM context,
                                          size_t nonceSize,
                                          void const *pNonce,
                                          size_t encryptedDataSize,
                                          void const *pEncryptedData,
                                          size_t *pDecryptedDataSize,
                                          void *pDecryptedData) {
    if (nonceSize != FOE_CRYPTO_AES_256_GCM_NONCE_SIZE)
        return to_foeResult(FOE_CRYPTO_ERROR_INVALID_NONCE_SIZE);

    // Make sure the destination/decrypted buffer is large enough
    if (encryptedDataSize > *pDecryptedDataSize + FOE_CRYPTO_AES_256_GCM_ENCRYPTION_OVERHEAD)
        return to_foeResult(FOE_CRYPTO_ERROR_DESTINATION_BUFFER_TOO_SMALL);

    unsigned long long actualDecryptedDataSize;
    if (crypto_aead_aes256gcm_decrypt_afternm(pDecryptedData, &actualDecryptedDataSize, NULL,
                                              pEncryptedData, encryptedDataSize, NULL, 0, pNonce,
                                              encryption_state_from_handle(context)) != 0) {
        return to_foeResult(FOE_CRYPTO_ERROR_FAILED_TO_DECRYPT);
    }

    *pDecryptedDataSize = actualDecryptedDataSize;
    return to_foeResult(FOE_CRYPTO_SUCCESS);
}