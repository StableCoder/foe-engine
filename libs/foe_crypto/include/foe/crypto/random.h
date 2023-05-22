// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_CRYPTO_RANDOM_H
#define FOE_CRYPTO_RANDOM_H

#include <foe/crypto/export.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_CRYPTO_EXPORT
void foeCryptoGenerateRandomData(size_t size, void *pData);

#ifdef __cplusplus
}
#endif

#endif // FOE_CRYPTO_RANDOM_H