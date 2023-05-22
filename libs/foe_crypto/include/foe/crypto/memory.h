// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_CRYPTO_MEMORY_H
#define FOE_CRYPTO_MEMORY_H

#include <foe/crypto/export.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_CRYPTO_EXPORT
void foeCryptoZeroMemory(size_t size, void *pMemory);

#ifdef __cplusplus
}
#endif

#endif // FOE_CRYPTO_MEMORY_H