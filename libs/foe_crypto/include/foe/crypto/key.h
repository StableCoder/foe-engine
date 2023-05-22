// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_CRYPTO_KEY_H
#define FOE_CRYPTO_KEY_H

#include <foe/crypto/export.h>
#include <foe/handle.h>
#include <foe/result.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeCryptoKey)

FOE_CRYPTO_EXPORT
foeResultSet foeCreateCryptoKey(size_t keySize, void const *pKeyData, foeCryptoKey *pKey);

FOE_CRYPTO_EXPORT
void foeDestroyCryptoKey(foeCryptoKey key);

FOE_CRYPTO_EXPORT
size_t foeCryptoGetKeySize(foeCryptoKey key);

FOE_CRYPTO_EXPORT
void const *foeCryptoGetKeyData(foeCryptoKey key);

#ifdef __cplusplus
}
#endif

#endif // FOE_CRYPTO_KEY_H