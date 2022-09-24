// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/managed_memory.h>

#include "result.h"

typedef struct ManagedMemorySubset {
    foeManagedMemory parentMemory;
} ManagedMemorySubset;

static void cleanup_ManagedMemorySubset(void *pData, uint32_t dataSize, void *pMetadata) {
    ManagedMemorySubset *pManagedMemorySubsetData = (ManagedMemorySubset *)pMetadata;

    foeManagedMemoryDecrementUse(pManagedMemorySubsetData->parentMemory);
}

foeResultSet foeCreateManagedMemorySubset(foeManagedMemory parentMemory,
                                          size_t dataOffset,
                                          size_t dataSize,
                                          foeManagedMemory *pManagedMemory) {
    uint8_t *pParentData;
    uint32_t parentDataSize;

    foeManagedMemoryGetData(parentMemory, (void **)&pParentData, &parentDataSize);

    if (dataOffset + dataSize > parentDataSize) {
        return to_foeResult(FOE_ERROR_MEMORY_SUBSET_OVERRUNS_PARENT);
    }

    pParentData += dataOffset;

    ManagedMemorySubset metadata = {
        .parentMemory = parentMemory,
    };

    foeResultSet result =
        foeCreateManagedMemory(pParentData, dataSize, cleanup_ManagedMemorySubset, &metadata,
                               sizeof(ManagedMemorySubset), pManagedMemory);
    if (result.value == FOE_SUCCESS) {
        foeManagedMemoryIncrementUse(parentMemory);
    }

    return result;
}