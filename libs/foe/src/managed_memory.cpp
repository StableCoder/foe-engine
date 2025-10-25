// Copyright (C) 2022-2025 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/managed_memory.h>

#include "result.h"

#include <atomic>
#include <new>

#include <stdlib.h>
#include <string.h>

namespace {

struct ManagedMemory {
    void *pData;
    size_t dataSize;

    std::atomic_uint32_t useCount;
    PFN_foeManagedMemoryCleanup cleanupFn;

    ManagedMemory(void *pData, size_t dataSize, PFN_foeManagedMemoryCleanup cleanupFn) :
        pData{pData}, dataSize{dataSize}, useCount{1}, cleanupFn{cleanupFn} {}
};

FOE_DEFINE_HANDLE_CASTS(managed_memory, ManagedMemory, foeManagedMemory)

void *foeResourceCreateInfoGetMetadata(ManagedMemory const *pManagedMemory) {
    return (char *)pManagedMemory + sizeof(ManagedMemory);
}

} // namespace

extern "C" foeResultSet foeCreateManagedMemory(void *pData,
                                               size_t dataSize,
                                               PFN_foeManagedMemoryCleanup cleanupFn,
                                               void *pMetadata,
                                               size_t metadataSize,
                                               foeManagedMemory *pManagedMemory) {
    ManagedMemory *pNewManagedMemory =
        (ManagedMemory *)malloc(sizeof(ManagedMemory) + metadataSize);
    if (pNewManagedMemory == nullptr)
        return to_foeResult(FOE_ERROR_OUT_OF_MEMORY);

    new (pNewManagedMemory) ManagedMemory(pData, dataSize, cleanupFn);

    if (metadataSize != 0)
        memcpy(foeResourceCreateInfoGetMetadata(pNewManagedMemory), pMetadata, metadataSize);

    *pManagedMemory = managed_memory_to_handle(pNewManagedMemory);

    return to_foeResult((foeResult)FOE_SUCCESS);
}

extern "C" void foeManagedMemoryGetData(foeManagedMemory managedMemory,
                                        void **ppData,
                                        uint32_t *pDataSize) {
    ManagedMemory *pManagedMemory = managed_memory_from_handle(managedMemory);

    *ppData = pManagedMemory->pData;
    if (pDataSize)
        *pDataSize = pManagedMemory->dataSize;
}

extern "C" uint32_t foeManagedMemoryGetUse(foeManagedMemory managedMemory) {
    ManagedMemory *pManagedMemory = managed_memory_from_handle(managedMemory);

    return pManagedMemory->useCount;
}

extern "C" uint32_t foeManagedMemoryIncrementUse(foeManagedMemory managedMemory) {
    ManagedMemory *pManagedMemory = managed_memory_from_handle(managedMemory);

    return ++pManagedMemory->useCount;
}

extern "C" uint32_t foeManagedMemoryDecrementUse(foeManagedMemory managedMemory) {
    ManagedMemory *pManagedMemory = managed_memory_from_handle(managedMemory);

    uint32_t count = --pManagedMemory->useCount;
    if (count == 0) {
        if (pManagedMemory->cleanupFn)
            pManagedMemory->cleanupFn(pManagedMemory->pData, pManagedMemory->dataSize,
                                      foeResourceCreateInfoGetMetadata(pManagedMemory));

        pManagedMemory->~ManagedMemory();
        free(pManagedMemory);
    }

    return count;
}