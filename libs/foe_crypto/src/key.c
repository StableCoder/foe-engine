// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/crypto/key.h>

#include <foe/crypto/memory.h>
#include <sodium/core.h>
#include <sodium/utils.h>

#include "result.h"

#include <string.h>

typedef struct KeyDescription {
    uint32_t size;
} KeyDescription;

FOE_DEFINE_HANDLE_CASTS(key, KeyDescription, foeCryptoKey)

foeResultSet foeCreateCryptoKey(size_t keySize, void const *pKeyData, foeCryptoKey *pKey) {
    if (keySize == 0)
        return to_foeResult(FOE_CRYPTO_ERROR_INVALID_KEY_SIZE);

    if (sodium_init() < 0)
        return to_foeResult(FOE_CRYPTO_ERROR_LIBRARY_FAILED_TO_INITIALIZE);

    KeyDescription *pNewKey = sodium_malloc(sizeof(KeyDescription) + keySize);
    if (pNewKey == NULL)
        return to_foeResult(FOE_CRYPTO_ERROR_OUT_OF_MEMORY);

    pNewKey->size = keySize;

    // Copy the key data
    // The key memory has not been marked as read-only yet
    memcpy((void *)foeCryptoGetKeyData(key_to_handle(pNewKey)), pKeyData, keySize);

    // Set the key memory to be read-only
    sodium_mprotect_readonly(pNewKey);

    *pKey = key_to_handle(pNewKey);
    return to_foeResult(FOE_CRYPTO_SUCCESS);
}

void foeDestroyCryptoKey(foeCryptoKey key) { sodium_free(key); }

size_t foeCryptoGetKeySize(foeCryptoKey key) {
    KeyDescription *pKey = key_from_handle(key);

    return pKey->size;
}

void const *foeCryptoGetKeyData(foeCryptoKey key) {
    uint8_t *pKey = (uint8_t *)key_from_handle(key);
    pKey += sizeof(KeyDescription);

    return pKey;
}