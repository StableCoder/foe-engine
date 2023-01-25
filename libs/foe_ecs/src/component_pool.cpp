// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/ecs/component_pool.h>

#include <foe/algorithm.hpp>

#include "result.h"

#include <algorithm>
#include <atomic>
#include <cstring>
#include <mutex>
#include <vector>

namespace {

struct DataSet {
    size_t capacity;
    size_t count;
    foeEntityID *pIDs;
    void *pData;
};

struct InsertOffsets {
    foeEntityID entity;
    size_t srcOffset;
    size_t dstOffset;
};

struct ComponentPool {
    // Regular Pool
    DataSet storedData;

    // To Insert
    std::mutex toInsertSync;
    DataSet toInsertData;
    std::vector<InsertOffsets> toInsertOffsets;
    // Inserted
    size_t insertedCapacity;
    size_t insertedCount;
    size_t *pInsertedOffsets;

    // To Remove
    std::mutex toRemoveSync;
    std::vector<foeEntityID> toRemoveIDs;
    // Removed
    DataSet removedData;

    // Other Data
    size_t dataSize;
    PFN_foeEcsComponentDestructor
        dataDestructor;           // Function to call when component data is being destroyed
    size_t expansionRate;         // The linear rate at which the storage capacity increases
    size_t desiredCapacity;       // The desired minimum capacity, enforced during maintenance cycle
    size_t desiredInsertCapacity; // Desired capacity to items toInsert, to reduce reallocations
};

FOE_DEFINE_HANDLE_CASTS(component_pool, ComponentPool, foeEcsComponentPool)

void deallocDataSet(DataSet *pDataSet) {
    free(pDataSet->pData);
    free(pDataSet->pIDs);
}

bool allocDataSet(size_t capacity, size_t dataSize, DataSet *pDataSet) {
    DataSet newDataSet = {
        .capacity = capacity,
        .pIDs = (foeEntityID *)malloc(capacity * sizeof(foeEntityID)),
        .pData = malloc(capacity * dataSize),
    };

    if (newDataSet.pIDs == nullptr || newDataSet.pData == nullptr) {
        deallocDataSet(pDataSet);
        return false;
    } else {
        *pDataSet = newDataSet;
        return true;
    }
}

void moveData(DataSet *pDstPool,
              size_t dstOffset,
              DataSet *pSrcPool,
              size_t srcOffset,
              size_t count,
              size_t dataSize) {
    // IDs
    memmove(pDstPool->pIDs + dstOffset, pSrcPool->pIDs + srcOffset, count * sizeof(foeEntityID));
    // Data
    memmove((uint8_t *)(pDstPool->pData) + dstOffset * dataSize,
            (uint8_t *)(pSrcPool->pData) + srcOffset * dataSize, count * dataSize);
}

foeResultSet removePass(ComponentPool *pComponentPool) {
    // Acquire lock, and get the data we're going to use, then unlock it to allow
    // new incoming removals
    pComponentPool->toRemoveSync.lock();

    auto toRemoveIDs = std::move(pComponentPool->toRemoveIDs);

    pComponentPool->toRemoveSync.unlock();

    // If there aren't any to remove, leave early
    if (toRemoveIDs.empty())
        return to_foeResult(FOE_ECS_SUCCESS);

    // Sort the IDs to be in-order
    std::sort(toRemoveIDs.begin(), toRemoveIDs.end());

    // Prepare removed storage for new items, make sure it has enough capacity
    if (pComponentPool->removedData.capacity < toRemoveIDs.size()) {
        deallocDataSet(&pComponentPool->removedData);
        pComponentPool->removedData = {};

        if (!allocDataSet(toRemoveIDs.size(), pComponentPool->dataSize,
                          &pComponentPool->removedData))
            return to_foeResult(FOE_ECS_ERROR_OUT_OF_MEMORY);
    }

    foeEntityID const *pID = pComponentPool->storedData.pIDs;
    foeEntityID const *const pStartID = pID;
    foeEntityID const *const pEndID = pStartID + pComponentPool->storedData.count;

    foeEntityID const *pRemoveID = toRemoveIDs.data();
    foeEntityID const *const pEndRemoveID = pRemoveID + toRemoveIDs.size();

    size_t removedCount = 0;
    size_t lastShiftedOffset = 0;

    while (pID != pEndID && pRemoveID != pEndRemoveID) {
        // If we haven't found the next item to remove yet, continue the search
        pID = std::lower_bound(pID, pEndID, *pRemoveID);
        if (pID == pEndID)
            break;

        if (*pID == *pRemoveID) {
            // Move the removed entry out
            size_t const offset = pID - pStartID;
            moveData(&pComponentPool->removedData, removedCount, &pComponentPool->storedData,
                     offset, 1, pComponentPool->dataSize);

            // Move any other objects that need to be shifted down in the regular storage
            if (lastShiftedOffset != 0) {
                moveData(&pComponentPool->storedData, lastShiftedOffset - removedCount,
                         &pComponentPool->storedData, lastShiftedOffset, offset - lastShiftedOffset,
                         pComponentPool->dataSize);
            }

            ++pID;
            ++pRemoveID;
            ++removedCount;
            lastShiftedOffset = offset + 1;
        } else {
            // The ID is larger than the ID to remove, so the ID isn't in the pool, so skip it
            ++pRemoveID;
        }
    }

    // Need to move any remaining objects
    if (lastShiftedOffset != 0) {
        moveData(&pComponentPool->storedData, lastShiftedOffset - removedCount,
                 &pComponentPool->storedData, lastShiftedOffset,
                 pComponentPool->storedData.count - lastShiftedOffset, pComponentPool->dataSize);
    }

    pComponentPool->storedData.count -= removedCount;
    pComponentPool->removedData.count = removedCount;

    return to_foeResult(FOE_ECS_SUCCESS);
}

foeResultSet insertPass(ComponentPool *pComponentPool) {
    // Acquire lock, and get the data we're going to use, then unlock it to allow
    // new incoming insertions
    pComponentPool->toInsertSync.lock();

    DataSet toInsertDataSet = pComponentPool->toInsertData;
    pComponentPool->toInsertData = {};
    std::vector<InsertOffsets> toInsertOffsets = std::move(pComponentPool->toInsertOffsets);

    pComponentPool->toInsertSync.unlock();

    // If there's nothing to actually insert, leave early
    if (toInsertDataSet.count == 0) {
        deallocDataSet(&toInsertDataSet);
        return to_foeResult(FOE_ECS_SUCCESS);
    }

    { // Sort data to insert
        std::sort(
            toInsertOffsets.begin(), toInsertOffsets.end(),
            [](InsertOffsets const &a, InsertOffsets const &b) { return a.entity < b.entity; });

        auto newToInsertOffsetsEnd = foe::unique_last(
            toInsertOffsets.begin(), toInsertOffsets.end(),
            [](InsertOffsets const &a, InsertOffsets const &b) { return a.entity == b.entity; });

        if (pComponentPool->dataDestructor) {
            for (auto it = newToInsertOffsetsEnd; it != toInsertOffsets.end(); ++it) {
                void *const pSkippedData =
                    (uint8_t *)toInsertDataSet.pData + (it->srcOffset * pComponentPool->dataSize);
                pComponentPool->dataDestructor(pSkippedData);
            }
        }

        toInsertOffsets.erase(newToInsertOffsetsEnd, toInsertOffsets.end());
    }

    // Insertion offset pointers
    InsertOffsets *pInsert = toInsertOffsets.data();
    InsertOffsets const *const pInsertEnd = pInsert + toInsertOffsets.size();

    // Regular storage ID iterators
    foeEntityID const *pID = pComponentPool->storedData.pIDs;
    foeEntityID const *const pStartID = pID;
    foeEntityID const *const pEndID = pID + pComponentPool->storedData.count;

    size_t toInsertCount = 0;
    while (pInsert != pInsertEnd) {
        // Try to shortcut a linear search by doing binary search
        pID = std::lower_bound(pID, pEndID, pInsert->entity);

        // If the IDs match, meaning the item is already in the regular list, skip the insertion of
        // this element
        if (pID != pEndID && *pID == pInsert->entity) {
            // Mark the ID as invalid, so it is skipped when doing actual insertion pass later
            pInsert->entity = FOE_INVALID_ID;

            // If there's a destructor, be sure to call it on the data we're skipping
            if (pComponentPool->dataDestructor) {
                void *const pSkippedData = (uint8_t *)toInsertDataSet.pData +
                                           (pInsert->srcOffset * pComponentPool->dataSize);
                pComponentPool->dataDestructor(pSkippedData);
            }

            ++pInsert;
            continue;
        }

        // At this point, it is not already in the pool, is unique, so to be added
        pInsert->dstOffset = (pID - pStartID);

        ++toInsertCount;
        ++pInsert;
    }

    if (toInsertCount == 0) {
        deallocDataSet(&toInsertDataSet);
        return to_foeResult(FOE_ECS_SUCCESS);
    }

    // Resize data pool if necessary
    DataSet dstPool = pComponentPool->storedData;
    size_t newCapacity = std::max(pComponentPool->expansionRate, dstPool.count + toInsertCount);
    // Check if there's an external larger requested capacity
    if (pComponentPool->desiredCapacity > newCapacity)
        newCapacity = pComponentPool->desiredCapacity;

    if (dstPool.capacity < newCapacity) {
        if (!allocDataSet(newCapacity, pComponentPool->dataSize, &dstPool)) {
            if (pComponentPool->dataDestructor) {
                for (auto pInsert = toInsertOffsets.begin(); pInsert != toInsertOffsets.end();
                     ++pInsert) {
                    if (pInsert->entity != FOE_INVALID_ID) {
                        void *const pNonSkippedData =
                            (uint8_t *)toInsertDataSet.pData +
                            (pInsert->srcOffset * pComponentPool->dataSize);
                        pComponentPool->dataDestructor(pNonSkippedData);
                    }
                }
            }
            deallocDataSet(&toInsertDataSet);
            return to_foeResult(FOE_ECS_ERROR_OUT_OF_MEMORY);
        }
    }

    // Check capacity for inserted array
    if (pComponentPool->insertedCapacity < toInsertCount) {
        free(pComponentPool->pInsertedOffsets);
        pComponentPool->pInsertedOffsets = (size_t *)malloc(toInsertCount * sizeof(size_t));
        if (pComponentPool->pInsertedOffsets == nullptr) {
            if (pComponentPool->dataDestructor) {
                for (auto pInsert = toInsertOffsets.begin(); pInsert != toInsertOffsets.end();
                     ++pInsert) {
                    if (pInsert->entity != FOE_INVALID_ID) {
                        void *const pNonSkippedData =
                            (uint8_t *)toInsertDataSet.pData +
                            (pInsert->srcOffset * pComponentPool->dataSize);
                        pComponentPool->dataDestructor(pNonSkippedData);
                    }
                }
            }
            deallocDataSet(&toInsertDataSet);
            return to_foeResult(FOE_ECS_ERROR_OUT_OF_MEMORY);
        }
        pComponentPool->insertedCapacity = toInsertCount;
    }
    pComponentPool->insertedCount = toInsertCount;

    // Setup pointers for actual insertion pass
    pInsert = toInsertOffsets.data() + toInsertOffsets.size() - 1;
    InsertOffsets const *const pEndReverseInsert = toInsertOffsets.data() - 1;

    size_t *pInsertedOffset = pComponentPool->pInsertedOffsets + toInsertCount - 1;

    size_t lastShiftedOffset = pComponentPool->storedData.count;
    size_t shiftDistance = toInsertCount;

    // In reverse order, insert the items, shifting required items to the right
    for (; pInsert != pEndReverseInsert; --pInsert) {
        // Some items need to be skipped
        if (pInsert->entity == FOE_INVALID_ID)
            continue;

        if (pInsert->dstOffset < lastShiftedOffset) {
            size_t const srcOffset = pInsert->dstOffset;
            size_t const dstOffset = srcOffset + shiftDistance;
            size_t const numMove = lastShiftedOffset - srcOffset;

            moveData(&dstPool, dstOffset, &pComponentPool->storedData, srcOffset, numMove,
                     pComponentPool->dataSize);

            lastShiftedOffset = srcOffset;
        }

        // Everything necessary has been shifted out of the way, place new entry now
        --shiftDistance;

        moveData(&dstPool, pInsert->dstOffset + shiftDistance, &toInsertDataSet, pInsert->srcOffset,
                 1, pComponentPool->dataSize);

        *pInsertedOffset = pInsert->dstOffset + shiftDistance;
        --pInsertedOffset;
    }

    // If there is a new storage mediam, then move any remaining items from the old, and free the
    // old allocs
    dstPool.count = pComponentPool->storedData.count + toInsertCount;
    if (dstPool.pIDs != pComponentPool->storedData.pIDs) {
        // Move remaining old data
        if (0 < lastShiftedOffset) {
            moveData(&dstPool, 0, &pComponentPool->storedData, 0, lastShiftedOffset,
                     pComponentPool->dataSize);
        }

        // Dealloc
        deallocDataSet(&pComponentPool->storedData);
    }
    pComponentPool->storedData = dstPool;

    // Dealloc old toInsert
    deallocDataSet(&toInsertDataSet);

    return to_foeResult(FOE_ECS_SUCCESS);
}

} // namespace

extern "C" foeResultSet foeEcsCreateComponentPool(size_t initialCapacity,
                                                  size_t expansionRate,
                                                  size_t dataSize,
                                                  PFN_foeEcsComponentDestructor dataDestructor,
                                                  foeEcsComponentPool *pComponentPool) {
    ComponentPool *pNewComponentPool = (ComponentPool *)calloc(1, sizeof(ComponentPool));
    if (pNewComponentPool == nullptr)
        return to_foeResult(FOE_ECS_ERROR_OUT_OF_MEMORY);

    // Initialize in-place for C++ elements
    new (pNewComponentPool) ComponentPool;

    pNewComponentPool->dataSize = dataSize;
    pNewComponentPool->dataDestructor = dataDestructor;
    pNewComponentPool->expansionRate = expansionRate;
    pNewComponentPool->desiredCapacity = initialCapacity;

    *pComponentPool = component_pool_to_handle(pNewComponentPool);

    return to_foeResult(FOE_ECS_SUCCESS);
}

extern "C" void foeEcsDestroyComponentPool(foeEcsComponentPool componentPool) {
    ComponentPool *pComponentPool = component_pool_from_handle(componentPool);

    if (pComponentPool->dataDestructor != NULL) {
        uint8_t *pData = (uint8_t *)pComponentPool->removedData.pData;
        for (size_t i = 0; i < pComponentPool->removedData.count;
             ++i, pData += pComponentPool->dataSize) {
            pComponentPool->dataDestructor(pData);
        }

        pData = (uint8_t *)pComponentPool->toInsertData.pData;
        for (size_t i = 0; i < pComponentPool->toInsertData.count;
             ++i, pData += pComponentPool->dataSize) {
            pComponentPool->dataDestructor(pData);
        }

        pData = (uint8_t *)pComponentPool->storedData.pData;
        for (size_t i = 0; i < pComponentPool->storedData.count;
             ++i, pData += pComponentPool->dataSize) {
            pComponentPool->dataDestructor(pData);
        }
    }

    deallocDataSet(&pComponentPool->removedData);

    deallocDataSet(&pComponentPool->toInsertData);
    free(pComponentPool->pInsertedOffsets);

    deallocDataSet(&pComponentPool->storedData);

    pComponentPool->~ComponentPool();
    free(pComponentPool);
}

extern "C" foeResultSet foeEcsComponentPoolMaintenance(foeEcsComponentPool componentPool) {
    ComponentPool *pComponentPool = component_pool_from_handle(componentPool);

    // Clear Removed/Inserted
    if (pComponentPool->dataDestructor != NULL) {
        uint8_t *pData = (uint8_t *)pComponentPool->removedData.pData;
        for (size_t i = 0; i < pComponentPool->removedData.count;
             ++i, pData += pComponentPool->dataSize) {
            pComponentPool->dataDestructor(pData);
        }
    }
    pComponentPool->removedData.count = 0;
    pComponentPool->insertedCount = 0;

    foeResultSet result = removePass(pComponentPool);
    if (result.value != FOE_SUCCESS)
        return result;

    result = insertPass(pComponentPool);

    return result;
}

extern "C" void foeEcsComponentPoolExpansionRate(foeEcsComponentPool componentPool,
                                                 size_t expansionRate) {
    ComponentPool *pComponentPool = component_pool_from_handle(componentPool);

    pComponentPool->expansionRate = expansionRate;
}

extern "C" size_t foeEcsComponentPoolSize(foeEcsComponentPool componentPool) {
    ComponentPool *pComponentPool = component_pool_from_handle(componentPool);

    return pComponentPool->storedData.count;
}

extern "C" foeEntityID const *foeEcsComponentPoolIdPtr(foeEcsComponentPool componentPool) {
    ComponentPool *pComponentPool = component_pool_from_handle(componentPool);

    return (foeEntityID *)pComponentPool->storedData.pIDs;
}

extern "C" void *foeEcsComponentPoolDataPtr(foeEcsComponentPool componentPool) {
    ComponentPool *pComponentPool = component_pool_from_handle(componentPool);

    return pComponentPool->storedData.pData;
}

extern "C" size_t foeEcsComponentPoolCapacity(foeEcsComponentPool componentPool) {
    ComponentPool *pComponentPool = component_pool_from_handle(componentPool);

    return pComponentPool->storedData.capacity;
}

extern "C" void foeEcsComponentPoolReserve(foeEcsComponentPool componentPool, size_t capacity) {
    ComponentPool *pComponentPool = component_pool_from_handle(componentPool);

    pComponentPool->desiredCapacity = std::max(pComponentPool->desiredCapacity, capacity);
}

extern "C" size_t foeEcsComponentPoolInsertCapacity(foeEcsComponentPool componentPool) {
    ComponentPool *pComponentPool = component_pool_from_handle(componentPool);

    return pComponentPool->toInsertData.capacity;
}

extern "C" void foeEcsComponentPoolReserveInsertCapacity(foeEcsComponentPool componentPool,
                                                         size_t capacity) {
    ComponentPool *pComponentPool = component_pool_from_handle(componentPool);

    pComponentPool->desiredInsertCapacity =
        std::max(pComponentPool->desiredInsertCapacity, capacity);
}

extern "C" foeResultSet foeEcsComponentPoolInsert(foeEcsComponentPool componentPool,
                                                  foeEntityID entity,
                                                  void *pData) {
    ComponentPool *pComponentPool = component_pool_from_handle(componentPool);
    std::unique_lock toInsertLock{pComponentPool->toInsertSync};

    // Check if we need to expand the capacity of the toInsert data pool
    size_t newCapacity = 0;
    if (pComponentPool->desiredInsertCapacity != 0) {
        newCapacity = pComponentPool->desiredInsertCapacity;
        pComponentPool->desiredInsertCapacity = 0;
    }
    if (newCapacity < pComponentPool->toInsertData.capacity + 1) {
        newCapacity = pComponentPool->toInsertData.capacity + 16;
    }

    if (pComponentPool->toInsertData.capacity < newCapacity) {
        DataSet newDataSet = {};
        if (!allocDataSet(newCapacity, pComponentPool->dataSize, &newDataSet))
            return to_foeResult(FOE_ECS_ERROR_OUT_OF_MEMORY);

        moveData(&newDataSet, 0, &pComponentPool->toInsertData, 0,
                 pComponentPool->toInsertData.count, pComponentPool->dataSize);

        deallocDataSet(&pComponentPool->toInsertData);
        newDataSet.count = pComponentPool->toInsertData.count;
        pComponentPool->toInsertData = newDataSet;
    }

    // Actually add the data to be inserted
    pComponentPool->toInsertData.pIDs[pComponentPool->toInsertData.count] = entity;
    memcpy((uint8_t *)pComponentPool->toInsertData.pData +
               (pComponentPool->toInsertData.count * pComponentPool->dataSize),
           pData, pComponentPool->dataSize);

    pComponentPool->toInsertOffsets.emplace_back(InsertOffsets{
        .entity = entity,
        .srcOffset = pComponentPool->toInsertData.count,
    });

    ++pComponentPool->toInsertData.count;

    return to_foeResult(FOE_ECS_SUCCESS);
}

extern "C" size_t foeEcsComponentPoolInserted(foeEcsComponentPool componentPool) {
    ComponentPool *pComponentPool = component_pool_from_handle(componentPool);

    return pComponentPool->insertedCount;
}

extern "C" size_t const *foeEcsComponentPoolInsertedOffsetPtr(foeEcsComponentPool componentPool) {
    ComponentPool *pComponentPool = component_pool_from_handle(componentPool);

    return pComponentPool->pInsertedOffsets;
}

extern "C" foeResultSet foeEcsComponentPoolRemove(foeEcsComponentPool componentPool,
                                                  foeEntityID entity) {
    ComponentPool *pComponentPool = component_pool_from_handle(componentPool);

    pComponentPool->toRemoveSync.lock();

    pComponentPool->toRemoveIDs.emplace_back(entity);

    pComponentPool->toRemoveSync.unlock();

    return to_foeResult(FOE_ECS_SUCCESS);
}

extern "C" size_t foeEcsComponentPoolRemoved(foeEcsComponentPool componentPool) {
    ComponentPool *pComponentPool = component_pool_from_handle(componentPool);

    return pComponentPool->removedData.count;
}

extern "C" foeEntityID const *foeEcsComponentPoolRemovedIdPtr(foeEcsComponentPool componentPool) {
    ComponentPool *pComponentPool = component_pool_from_handle(componentPool);

    return pComponentPool->removedData.pIDs;
}

extern "C" void *foeEcsComponentPoolRemovedDataPtr(foeEcsComponentPool componentPool) {
    ComponentPool *pComponentPool = component_pool_from_handle(componentPool);

    return pComponentPool->removedData.pData;
}