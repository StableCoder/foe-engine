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

foeEcsIndexGenerator::foeEcsIndexGenerator(std::string_view name, foeGroupID groupId) :
    cName{name}, cGroupID{groupId}, mNumRecycled{0}, mRecycled{}, mNextFreeID{0} {
    /// \todo Replace with C++20 contracts
    assert((groupId & foeEcsValidIndexBits) == 0);
}

foeEntityID foeEcsIndexGenerator::generate() {
    foeEntityID entity = FOE_INVALID_ENTITY;

    std::scoped_lock lock{mSync};

    if (mNumRecycled != 0) {
        entity = mRecycled.front();
        mRecycled.pop();
        --mNumRecycled;
    } else if (mNextFreeID >= foeEcsInvalidIndexID) {
        // Ran out of indexes for this group
        return FOE_INVALID_ENTITY;
    } else {
        // Fresh index
        entity = mNextFreeID++;
    }

    return cGroupID | entity;
}

bool foeEcsIndexGenerator::free(foeEntityID entity) { return free(1, &entity); }

bool foeEcsIndexGenerator::free(uint32_t count, foeEntityID *pEntities) {
    // Validate that all the IDs to be recycled belong to this generator
    auto const *const endIt = pEntities + count;
    for (auto const *it = pEntities; it != endIt; ++it) {
        if (*it == FOE_INVALID_ENTITY || foeEcsGetGroupID(*it) != cGroupID ||
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

foeGroupID foeEcsIndexGenerator::groupID() const noexcept { return cGroupID; }

foeIndexID foeEcsIndexGenerator::peekNextFreshIndex() const noexcept { return mNextFreeID; }

size_t foeEcsIndexGenerator::recyclable() const noexcept { return mNumRecycled; }

void foeEcsIndexGenerator::importState(foeIndexID nextIndex,
                                       const std::vector<foeIndexID> &recycledIndices) {
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

void foeEcsIndexGenerator::exportState(foeIndexID &nextIndex,
                                       std::vector<foeIndexID> &recycledIndices) {
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