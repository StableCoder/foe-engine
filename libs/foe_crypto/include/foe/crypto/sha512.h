// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_CRYPTO_SHA512_H
#define FOE_CRYPTO_SHA512_H

#include <foe/crypto/export.h>

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FOE_CRYPTO_SHA512_HASH_SIZE 64U

// pHash mush be FOE_CRYPTO_SHA512_HASH_SIZE bytes
FOE_CRYPTO_EXPORT
bool foeCryptoHashSHA512(size_t size, void const *pData, void *pHash);

#ifdef __cplusplus
}
#endif

#endif // FOE_CRYPTO_SHA512_H