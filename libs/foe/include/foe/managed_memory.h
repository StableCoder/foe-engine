// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_MANAGED_MEMORY_H
#define FOE_MANAGED_MEMORY_H

#include <foe/export.h>
#include <foe/handle.h>
#include <foe/result.h>

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeManagedMemory)

typedef void (*PFN_foeManagedMemoryCleanup)(void *pData, uint32_t dataSize, void *pMetadata);

FOE_EXPORT
foeResultSet foeCreateManagedMemory(void *pData,
                                    size_t dataSize,
                                    PFN_foeManagedMemoryCleanup cleanupFn,
                                    void *pMetadata,
                                    size_t metadataSize,
                                    foeManagedMemory *pManagedMemory);

FOE_EXPORT
foeResultSet foeCreateManagedMemorySubset(foeManagedMemory parentMemory,
                                          size_t dataOffset,
                                          size_t dataSize,
                                          foeManagedMemory *pManagedMemory);

FOE_EXPORT
void foeManagedMemoryGetData(foeManagedMemory managedMemory, void **ppData, uint32_t *pDataSize);

FOE_EXPORT
uint32_t foeManagedMemoryGetUse(foeManagedMemory managedMemory);

FOE_EXPORT
uint32_t foeManagedMemoryIncrementUse(foeManagedMemory managedMemory);

FOE_EXPORT
uint32_t foeManagedMemoryDecrementUse(foeManagedMemory managedMemory);

#ifdef __cplusplus
}
#endif

#endif // FOE_MANAGED_MEMORY_H