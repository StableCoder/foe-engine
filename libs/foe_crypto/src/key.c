// Copyright (C) 2023-2025 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/crypto/key.h>

#include <foe/crypto/memory.h>
#include <sodium/core.h>
#include <sodium/utils.h>

#include "result.h"

#include <stdio.h>
#include <string.h>

// follow machine size
typedef size_t KeySizeType;

foeResultSet foeCreateCryptoKey(size_t keySize, void const *pKeyData, foeCryptoKey *pKey) {
    if (keySize == 0)
        return to_foeResult(FOE_CRYPTO_ERROR_INVALID_KEY_SIZE);

    if (sodium_init() < 0)
        return to_foeResult(FOE_CRYPTO_ERROR_LIBRARY_FAILED_TO_INITIALIZE);

    void *pNewKey = sodium_malloc(sizeof(uint8_t) + (2 * sizeof(KeySizeType)) + keySize);
    if (pNewKey == NULL)
        return to_foeResult(FOE_CRYPTO_ERROR_OUT_OF_MEMORY);

    // to deal with alignment issues, the first byte indicates how many bytes until the start of the
    // key size, and the key data is always `sizeof(uint8_t) + (2*sizeof(KeySizeType))` bytes after
    // the original key pointer
    size_t totalPoint = (size_t)pNewKey;
    uint8_t neededOffset = 8 - (size_t)pNewKey % sizeof(KeySizeType);
    *(uint8_t *)pNewKey = neededOffset;

    uint8_t sizeOffset = *(uint8_t *)pNewKey;

    KeySizeType *pNewKeySize = (size_t *)(((uint8_t *)pNewKey) + neededOffset);
    *pNewKeySize = keySize;

    // Copy the key data
    // The key memory has not been marked as read-only yet
    memcpy((void *)foeCryptoGetKeyData(pNewKey), pKeyData, keySize);

    // Set the key memory to be read-only
    sodium_mprotect_readonly(pNewKey);

    *pKey = pNewKey;
    return to_foeResult(FOE_CRYPTO_SUCCESS);
}

void foeDestroyCryptoKey(foeCryptoKey key) { sodium_free(key); }

size_t foeCryptoGetKeySize(foeCryptoKey key) {
    uint8_t sizeOffset = *(uint8_t *)key;
    KeySizeType *size = (KeySizeType *)((uint8_t *)key + sizeOffset);

    return *size;
}

void const *foeCryptoGetKeyData(foeCryptoKey key) {
    uint8_t *pKey = (uint8_t *)key;
    pKey += sizeof(uint8_t) + (2 * sizeof(KeySizeType));

    return pKey;
}