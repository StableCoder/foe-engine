// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/crypto/memory.h>

#include <sodium/utils.h>

void foeCryptoZeroMemory(size_t size, void *pMemory) { sodium_memzero(pMemory, size); }