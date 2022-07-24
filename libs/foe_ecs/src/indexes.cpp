// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/ecs/indexes.h>

#include "result.h"

#include <algorithm>
#include <atomic>
#include <mutex>
#include <queue>

namespace {

struct Indexes {
    /// The group of the entity IDs being managed
    foeIdGroup groupID;

    /// Synchronizes the recycle list
    std::mutex sync;
    /// The next free IndexID, never yet used.
    std::atomic<foeIdIndexValue> nextNewIndex;
    /// The list of recyclable IndexIDs.
    std::queue<foeIdIndex> recycled;
};

FOE_DEFINE_HANDLE_CASTS(indexes, Indexes, foeEcsIndexes)

} // namespace

extern "C" foeResultSet foeEcsCreateIndexes(foeIdGroup groupID, foeEcsIndexes *pIndexes) {
    if ((groupID & foeIdIndexBits) != 0) {
        return to_foeResult(FOE_ECS_ERROR_NOT_GROUP_ID);
    }

    Indexes *pNewIndexes = (Indexes *)malloc(sizeof(Indexes));
    if (pNewIndexes == nullptr)
        return to_foeResult(FOE_ECS_ERROR_OUT_OF_MEMORY);

    new (pNewIndexes) Indexes;

    pNewIndexes->groupID = groupID;
    pNewIndexes->nextNewIndex = foeIdIndexMinValue;

    *pIndexes = indexes_to_handle(pNewIndexes);

    return to_foeResult(FOE_ECS_SUCCESS);
}

extern "C" void foeEcsDestroyIndexes(foeEcsIndexes indexes) {
    Indexes *pIndexes = indexes_from_handle(indexes);

    pIndexes->~Indexes();
    free(pIndexes);
}

extern "C" foeIdGroup foeEcsIndexesGetGroupID(foeEcsIndexes indexes) {
    Indexes *pIndexes = indexes_from_handle(indexes);

    return pIndexes->groupID;
}

extern "C" foeResultSet foeEcsGenerateID(foeEcsIndexes indexes, foeId *pID) {
    Indexes *pIndexes = indexes_from_handle(indexes);

    std::unique_lock lock{pIndexes->sync};

    if (pIndexes->recycled.size() != 0) {
        *pID = foeIdCreate(pIndexes->groupID, pIndexes->recycled.front());
        pIndexes->recycled.pop();

        return to_foeResult(FOE_ECS_SUCCESS);
    } else if (pIndexes->nextNewIndex == foeIdIndexMaxValue) {
        // Ran out of indexes in the group
        return to_foeResult(FOE_ECS_ERROR_OUT_OF_INDEXES);
    }

    *pID = foeIdCreate(pIndexes->groupID, pIndexes->nextNewIndex++);
    return to_foeResult(FOE_ECS_SUCCESS);
}

extern "C" foeResultSet foeEcsFreeID(foeEcsIndexes indexes, foeId id) {
    return foeEcsFreeIDs(indexes, 1, &id);
}

extern "C" foeResultSet foeEcsFreeIDs(foeEcsIndexes indexes, uint32_t idCount, foeId const *pIDs) {
    Indexes *pIndexes = indexes_from_handle(indexes);

    // Validate that all the IDs are for the associated GroupID, and the IndexID is less than the
    // fresh index
    for (uint32_t i = 0; i < idCount; ++i) {
        if (pIDs[i] == FOE_INVALID_ID)
            return to_foeResult(FOE_ECS_ERROR_INVALID_ID);
        if (foeIdGetGroup(pIDs[i]) != pIndexes->groupID)
            return to_foeResult(FOE_ECS_ERROR_INCORRECT_GROUP_ID);
        if (foeIdGetIndex(pIDs[i]) >= pIndexes->nextNewIndex)
            return to_foeResult(FOE_ECS_ERROR_INDEX_ABOVE_GENERATED);
        if (foeIdGetIndex(pIDs[i]) < foeIdIndexMinValue)
            return to_foeResult(FOE_ECS_ERROR_INDEX_BELOW_MINIMUM);
    }

    std::unique_lock lock{pIndexes->sync};

    for (uint32_t i = 0; i < idCount; ++i)
        pIndexes->recycled.push(foeIdGetIndex(pIDs[i]));

    return to_foeResult(FOE_ECS_SUCCESS);
}

extern "C" void foeEcsForEachID(foeEcsIndexes indexes,
                                PFN_foeEcsForEachCall forEachCall,
                                void *pCallContext) {
    Indexes *pIndexes = indexes_from_handle(indexes);

    foeResultSet result;
    foeIdIndex nextNewIndex;
    std::vector<foeIdIndex> recycledIndexes;

    do {
        uint32_t count;
        foeEcsExportIndexes(indexes, nullptr, &count, nullptr);

        recycledIndexes.resize(count);
        result = foeEcsExportIndexes(indexes, &nextNewIndex, &count, recycledIndexes.data());
        recycledIndexes.resize(count);
    } while (result.value != FOE_SUCCESS);

    std::sort(recycledIndexes.begin(), recycledIndexes.end());

    foeIdIndex const *pRecycledIt = recycledIndexes.data();
    foeIdIndex const *const cRecycledEnd = pRecycledIt + recycledIndexes.size();

    for (foeIdIndex indexID = foeIdIndexMinValue; indexID < nextNewIndex; ++indexID) {
        // Skip through any recycled indexes
        while (pRecycledIt != cRecycledEnd && indexID == *pRecycledIt) {
            ++pRecycledIt;
            ++indexID;
        }

        if (indexID >= nextNewIndex)
            break;

        forEachCall(pCallContext, foeIdCreate(pIndexes->groupID, indexID));
    }
}

extern "C" foeResultSet foeEcsImportIndexes(foeEcsIndexes indexes,
                                            foeIdIndex nextNewIndex,
                                            uint32_t recycledCount,
                                            foeIdIndex const *pRecycledIndexes) {
    Indexes *pIndexes = indexes_from_handle(indexes);

    if (nextNewIndex < foeIdIndexMinValue) {
        return to_foeResult(FOE_ECS_ERROR_INDEX_BELOW_MINIMUM);
    }

    std::scoped_lock lock{pIndexes->sync};

    pIndexes->nextNewIndex = nextNewIndex;

    // Clear the old recycled list
    while (!pIndexes->recycled.empty())
        pIndexes->recycled.pop();

    // Insert new indices
    for (uint32_t i = 0; i < recycledCount; ++i) {
        pIndexes->recycled.emplace(pRecycledIndexes[i]);
    }

    return to_foeResult(FOE_ECS_SUCCESS);
}

extern "C" foeResultSet foeEcsExportIndexes(foeEcsIndexes indexes,
                                            foeIdIndex *pNextNewIndex,
                                            uint32_t *pRecycledCount,
                                            foeIdIndex *pRecycledIndexes) {
    Indexes *pIndexes = indexes_from_handle(indexes);

    foeResultSet result = to_foeResult(FOE_ECS_SUCCESS);
    std::unique_lock lock{pIndexes->sync};

    if (pNextNewIndex != nullptr)
        *pNextNewIndex = pIndexes->nextNewIndex;
    if (pRecycledIndexes == nullptr) {
        *pRecycledCount = pIndexes->recycled.size();
        return result;
    }

    auto tempList = pIndexes->recycled;
    lock.unlock();

    uint32_t returnCount = std::min((uint32_t)tempList.size(), *pRecycledCount);
    if (returnCount < tempList.size()) {
        result = to_foeResult(FOE_ECS_INCOMPLETE);
    }

    for (uint32_t i = 0; i < returnCount; ++i) {
        pRecycledIndexes[i] = tempList.front();
        tempList.pop();
    }

    return result;
}