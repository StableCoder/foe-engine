// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/crypto/sha512.h>

#include <sodium/crypto_hash_sha512.h>

bool foeCryptoHashSHA512(size_t size, void const *pData, void *pHash) {
    crypto_hash_sha512((unsigned char *)pHash, pData, size);

    return true;
}