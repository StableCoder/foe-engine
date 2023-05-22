// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/crypto/random.h>

#include <sodium/randombytes.h>

void foeCryptoGenerateRandomData(size_t dataSize, void *pData) { randombytes_buf(pData, dataSize); }