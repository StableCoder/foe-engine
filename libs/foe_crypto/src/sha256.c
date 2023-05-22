// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/crypto/sha256.h>

#include <sodium/crypto_hash_sha256.h>

bool foeCryptoHashSHA256(size_t size, void const *pData, void *pHash) {
    crypto_hash_sha256((unsigned char *)pHash, pData, size);

    return true;
}