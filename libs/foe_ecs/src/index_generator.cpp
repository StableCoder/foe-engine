/*
    Copyright (C) 2021-2022 George Cave.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <foe/ecs/index_generator.hpp>

#include "result.h"

#include <cassert>

foeIdIndexGenerator::foeIdIndexGenerator(foeIdGroup groupId) :
    cGroupID{groupId}, mNextFreeID{foeIdIndexMinValue}, mRecycled{}, mNumRecycled{0} {
    /// \todo Replace with C++20 contracts
    assert((groupId & foeIdIndexBits) == 0);
}

foeId foeIdIndexGenerator::generate() {
    foeId id = FOE_INVALID_ID;

    std::scoped_lock lock{mSync};

    if (mNumRecycled != 0) {
        id = mRecycled.front();
        mRecycled.pop();
        --mNumRecycled;
    } else if (mNextFreeID >= foeIdIndexMaxValue) {
        // Ran out of indexes for this group
        return FOE_INVALID_ID;
    } else {
        // Fresh index
        id = mNextFreeID++;
    }

    return foeIdCreate(cGroupID, id);
}

bool foeIdIndexGenerator::free(foeId id) { return free(1, &id); }

bool foeIdIndexGenerator::free(uint32_t count, foeId *pEntities) {
    // Validate that all the IDs to be recycled belong to this generator
    auto const *const endIt = pEntities + count;
    for (auto const *it = pEntities; it != endIt; ++it) {
        if (*it == FOE_INVALID_ID || foeIdGetGroup(*it) != cGroupID ||
            foeIdGetIndex(*it) >= mNextFreeID) {
            return false;
        }
    }

    // Add to recycled list
    std::scoped_lock lock{mSync};

    for (; pEntities != endIt; ++pEntities) {
        mRecycled.push(foeIdGetIndex(*pEntities));
    }

    mNumRecycled += count;

    return true;
}

foeIdGroup foeIdIndexGenerator::groupID() const noexcept { return cGroupID; }

foeIdIndex foeIdIndexGenerator::peekNextFreshIndex() const noexcept { return mNextFreeID; }

size_t foeIdIndexGenerator::recyclable() const noexcept { return mNumRecycled; }

void foeIdIndexGenerator::forEachID(std::function<void(foeId)> callFn) {
    foeResult result;
    foeIdIndex nextFreshIndex;
    std::vector<foeIdIndex> recycledIndexes;

    do {
        uint32_t count;
        exportState(nullptr, &count, nullptr);

        recycledIndexes.resize(count);
        result = exportState(&nextFreshIndex, &count, recycledIndexes.data());
        recycledIndexes.resize(count);
    } while (result.value != FOE_SUCCESS);

    std::sort(recycledIndexes.begin(), recycledIndexes.end());

    auto currentRecycled = recycledIndexes.begin();

    for (foeIdIndex indexID = foeIdIndexMinValue; indexID < nextFreshIndex; ++indexID) {
        // Skip through any recycled indexes
        while (currentRecycled != recycledIndexes.end() && indexID == *currentRecycled) {
            ++currentRecycled;
            ++indexID;
        }

        // Make sure we didn't go past the end (can happen if the last non-fresh index has been
        // recycled)
        if (indexID >= nextFreshIndex)
            break;

        callFn(foeIdCreate(cGroupID, indexID));
    }
}

foeResult foeIdIndexGenerator::importState(foeIdIndex nextIndex,
                                           uint32_t recycledCount,
                                           foeIdIndex const *pRecycledIndexes) {
    if (nextIndex < foeIdIndexMinValue) {
        return to_foeResult(FOE_ECS_ERROR_INDEX_BELOW_MINIMUM);
    }

    std::scoped_lock lock{mSync};

    mNextFreeID = nextIndex;
    mNumRecycled = recycledCount;

    // Clear the old recycled list
    while (!mRecycled.empty())
        mRecycled.pop();

    // Insert new indices
    for (uint32_t i = 0; i < recycledCount; ++i) {
        mRecycled.emplace(pRecycledIndexes[i]);
    }

    return to_foeResult(FOE_ECS_SUCCESS);
}

foeResult foeIdIndexGenerator::exportState(foeIdIndex *pNextIndex,
                                           uint32_t *pRecycledCount,
                                           foeIdIndex *pRecycledIndexes) {
    foeResult result = to_foeResult(FOE_ECS_SUCCESS);
    std::unique_lock lock{mSync};

    if (pNextIndex != nullptr)
        *pNextIndex = mNextFreeID;
    if (pRecycledIndexes == nullptr) {
        *pRecycledCount = mRecycled.size();
        return result;
    }

    auto tempList = mRecycled;
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
