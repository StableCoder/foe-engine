// Copyright (C) 2023-2024 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_CRYPTO_SHA256_H
#define FOE_CRYPTO_SHA256_H

#include <foe/crypto/export.h>

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FOE_CRYPTO_SHA256_HASH_SIZE 32

// pHash mush be FOE_CRYPTO_SHA256_HASH_SIZE bytes
FOE_CRYPTO_EXPORT
bool foeCryptoHashSHA256(size_t size, void const *pData, void *pHash);

#ifdef __cplusplus
}
#endif

#endif // FOE_CRYPTO_SHA256_H