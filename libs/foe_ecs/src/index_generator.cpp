/*
    Copyright (C) 2021 George Cave.

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

#include <cassert>

foeEcsIndexGenerator::foeEcsIndexGenerator(std::string_view name, foeIdGroup groupId) :
    cName{name}, cGroupID{groupId}, mNumRecycled{0}, mRecycled{}, mNextFreeID{0} {
    /// \todo Replace with C++20 contracts
    assert((groupId & foeEcsValidIndexBits) == 0);
}

foeId foeEcsIndexGenerator::generate() {
    foeId id = FOE_INVALID_ID;

    std::scoped_lock lock{mSync};

    if (mNumRecycled != 0) {
        id = mRecycled.front();
        mRecycled.pop();
        --mNumRecycled;
    } else if (mNextFreeID >= foeEcsInvalidIndexID) {
        // Ran out of indexes for this group
        return FOE_INVALID_ID;
    } else {
        // Fresh index
        id = mNextFreeID++;
    }

    return cGroupID | id;
}

foeId foeEcsIndexGenerator::generateResource() {
    auto newID = generate();
    if (newID != FOE_INVALID_ID) {
        newID |= foeEcsNumTypeBits;
    }
    return newID;
}

bool foeEcsIndexGenerator::free(foeId id) { return free(1, &id); }

bool foeEcsIndexGenerator::free(uint32_t count, foeId *pEntities) {
    // Validate that all the IDs to be recycled belong to this generator
    auto const *const endIt = pEntities + count;
    for (auto const *it = pEntities; it != endIt; ++it) {
        if (*it == FOE_INVALID_ID || foeEcsGetGroupID(*it) != cGroupID ||
            foeEcsGetIndexID(*it) >= mNextFreeID) {
            return false;
        }
    }

    // Add to recycled list
    std::scoped_lock lock{mSync};

    for (; pEntities != endIt; ++pEntities) {
        mRecycled.push(*pEntities);
    }

    mNumRecycled += count;

    return true;
}

std::string_view foeEcsIndexGenerator::name() const noexcept { return cName; }

foeIdGroup foeEcsIndexGenerator::groupID() const noexcept { return cGroupID; }

foeIdIndex foeEcsIndexGenerator::peekNextFreshIndex() const noexcept { return mNextFreeID; }

size_t foeEcsIndexGenerator::recyclable() const noexcept { return mNumRecycled; }

void foeEcsIndexGenerator::importState(foeIdIndex nextIndex,
                                       const std::vector<foeIdIndex> &recycledIndices) {
    std::scoped_lock lock{mSync};

    mNextFreeID = nextIndex;
    mNumRecycled = recycledIndices.size();

    // Clear the old recycled list
    while (!mRecycled.empty())
        mRecycled.pop();

    // Insert new indices
    for (auto const it : recycledIndices) {
        mRecycled.emplace(it);
    }
}

void foeEcsIndexGenerator::exportState(foeIdIndex &nextIndex,
                                       std::vector<foeIdIndex> &recycledIndices) {
    std::scoped_lock lock{mSync};

    nextIndex = mNextFreeID;

    auto tempList = mRecycled;

    recycledIndices.clear();
    recycledIndices.reserve(mNumRecycled);

    while (!tempList.empty()) {
        recycledIndices.emplace_back(tempList.front());
        tempList.pop();
    }
}

auto foeEcsIndexGenerator::activeEntityList() -> std::vector<foeId> {
    std::vector<foeId> entityList;

    // Generate list
    foeIdIndex nextFreeIndex;
    std::vector<foeIdIndex> recycled;
    exportState(nextFreeIndex, recycled);

    entityList.reserve(nextFreeIndex - recycled.size());
    std::sort(recycled.begin(), recycled.end());
    auto recycledIt = recycled.begin();

    for (foeIdIndex i = 0; i < nextFreeIndex; ++i) {
        if (recycledIt == recycled.end() || *recycledIt != i) {
            entityList.emplace_back(cGroupID | i);
        } else {
            ++recycledIt;
        }
    }

    return entityList;
}