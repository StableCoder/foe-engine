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

#ifndef FOE_ECS_DATA_POOL_IMPL_HPP
#define FOE_ECS_DATA_POOL_IMPL_HPP

#include <foe/algorithm.hpp>
#include <foe/data_pool.hpp>

#include <algorithm>
#include <cstring>

template <typename IdType, typename... Components>
foeDataPool<IdType, Components...>::foeDataPool(size_t expansionRate) :
    mExpansionRate{expansionRate},
    mMainStorage{},
    mStored{0},
    mToInsertSync{},
    mToInsert{},
    mInsertedOffsets{},
    mToRemoveSync{},
    mToRemove{},
    mRemovedStore{},
    mRemoved{0} {}

template <typename IdType, typename... Components>
foeDataPool<IdType, Components...>::~foeDataPool() {
    multiDelete(&mMainStorage, 0, size());
    clearRemoved();
}

template <typename IdType, typename... Components>
void foeDataPool<IdType, Components...>::maintenance() {
    clearRemoved();
    clearInserted();

    removePass();
    insertPass();
}

template <typename IdType, typename... Components>
void foeDataPool<IdType, Components...>::expansionRate(size_t expansionRate) {
    mExpansionRate = expansionRate;
}

template <typename IdType, typename... Components>
size_t foeDataPool<IdType, Components...>::expansionRate() const noexcept {
    return mExpansionRate;
}

template <typename IdType, typename... Components>
size_t foeDataPool<IdType, Components...>::capacity() const noexcept {
    return mMainStorage.capacity();
}

template <typename IdType, typename... Components>
size_t foeDataPool<IdType, Components...>::size() const noexcept {
    return mStored;
}

template <typename IdType, typename... Components>
size_t foeDataPool<IdType, Components...>::inserted() {
    return mInsertedOffsets.size();
}

template <typename IdType, typename... Components>
size_t foeDataPool<IdType, Components...>::removed() const noexcept {
    return mRemoved;
}

template <typename IdType, typename... Components>
bool foeDataPool<IdType, Components...>::exist(IdType id) const noexcept {
    return find(id) != size();
}

template <typename IdType, typename... Components>
void foeDataPool<IdType, Components...>::insert(IdType id, Components &&...components) {
    mToInsertSync.lock();

    mToInsert.emplace_back(id, std::make_tuple(std::move(components)...));

    mToInsertSync.unlock();
}

template <typename IdType, typename... Components>
void foeDataPool<IdType, Components...>::remove(IdType id) {
    mToRemoveSync.lock();
    mToRemove.emplace_back(id);
    mToRemoveSync.unlock();
}

template <typename IdType, typename... Components>
size_t foeDataPool<IdType, Components...>::sequential_search(IdType id) const noexcept {
    auto it = begin();
    auto const endIt = end();

    while (it != endIt) {
        if (*it == id) {
            return it - begin();
        }
        ++it;
    }

    return size();
}

template <typename IdType, typename... Components>
size_t foeDataPool<IdType, Components...>::binary_search(IdType id) const noexcept {
    auto searchIt = std::lower_bound(begin(), end(), id);
    if (searchIt != end() && *searchIt == id) {
        return searchIt - begin();
    }
    return size();
}

template <typename IdType, typename... Components>
size_t foeDataPool<IdType, Components...>::find(IdType id) const noexcept {
    auto searchIt = std::find(begin(), end(), id);
    return searchIt - begin();
}

template <typename IdType, typename... Components>
template <int Index>
auto foeDataPool<IdType, Components...>::begin() noexcept ->
    typename std::tuple_element<Index, TupleType>::type * {
    return static_cast<typename std::tuple_element<Index, TupleType>::type *>(
        mMainStorage.template get<Index>());
}

template <typename IdType, typename... Components>
template <int Index>
auto foeDataPool<IdType, Components...>::begin() const noexcept ->
    typename std::tuple_element<Index, TupleType>::type const * {
    return static_cast<typename std::tuple_element<Index, TupleType>::type const *>(
        mMainStorage.template get<Index>());
}

template <typename IdType, typename... Components>
template <int Index>
auto foeDataPool<IdType, Components...>::cbegin() const noexcept ->
    typename std::tuple_element<Index, TupleType>::type const * {
    return static_cast<typename std::tuple_element<Index, TupleType>::type const *>(
        mMainStorage.template get<Index>());
}

template <typename IdType, typename... Components>
template <int Index>
auto foeDataPool<IdType, Components...>::end() noexcept ->
    typename std::tuple_element<Index, TupleType>::type * {
    return static_cast<typename std::tuple_element<Index, TupleType>::type *>(
        mMainStorage.template get<Index>() + mStored);
}

template <typename IdType, typename... Components>
template <int Index>
auto foeDataPool<IdType, Components...>::end() const noexcept ->
    typename std::tuple_element<Index, TupleType>::type const * {
    return static_cast<typename std::tuple_element<Index, TupleType>::type const *>(
        mMainStorage.template get<Index>() + mStored);
}

template <typename IdType, typename... Components>
template <int Index>
auto foeDataPool<IdType, Components...>::cend() const noexcept ->
    typename std::tuple_element<Index, TupleType>::type const * {
    return static_cast<typename std::tuple_element<Index, TupleType>::type const *>(
        mMainStorage.template get<Index>() + mStored);
}

template <typename IdType, typename... Components>
auto foeDataPool<IdType, Components...>::inbegin() noexcept {
    return mInsertedOffsets.begin();
}

template <typename IdType, typename... Components>
auto foeDataPool<IdType, Components...>::inbegin() const noexcept {
    return mInsertedOffsets.begin();
}

template <typename IdType, typename... Components>
auto foeDataPool<IdType, Components...>::cinbegin() const noexcept {
    return mInsertedOffsets.cbegin();
}

template <typename IdType, typename... Components>
auto foeDataPool<IdType, Components...>::inend() noexcept {
    return mInsertedOffsets.end();
}

template <typename IdType, typename... Components>
auto foeDataPool<IdType, Components...>::inend() const noexcept {
    return mInsertedOffsets.end();
}

template <typename IdType, typename... Components>
auto foeDataPool<IdType, Components...>::cinend() const noexcept {
    return mInsertedOffsets.cend();
}

template <typename IdType, typename... Components>
size_t foeDataPool<IdType, Components...>::rm_sequential_search(IdType id) const noexcept {
    auto it = rmbegin();
    auto const endIt = rmend();

    while (it != endIt) {
        if (*it == id) {
            return it - rmbegin();
        }
        ++it;
    }

    return removed();
}

template <typename IdType, typename... Components>
size_t foeDataPool<IdType, Components...>::rm_binary_search(IdType id) const noexcept {
    auto searchIt = std::lower_bound(rmbegin(), rmend(), id);
    if (searchIt != rmend() && *searchIt == id) {
        return searchIt - rmbegin();
    }
    return removed();
}

template <typename IdType, typename... Components>
size_t foeDataPool<IdType, Components...>::rm_find(IdType id) const noexcept {
    auto searchIt = std::find(rmbegin(), rmend(), id);
    return searchIt - rmbegin();
}

template <typename IdType, typename... Components>
template <int Index>
auto foeDataPool<IdType, Components...>::rmbegin() noexcept ->
    typename std::tuple_element<Index, TupleType>::type * {
    return static_cast<typename std::tuple_element<Index, TupleType>::type *>(
        mRemovedStore.template get<Index>());
}

template <typename IdType, typename... Components>
template <int Index>
auto foeDataPool<IdType, Components...>::rmbegin() const noexcept ->
    typename std::tuple_element<Index, TupleType>::type const * {
    return static_cast<typename std::tuple_element<Index, TupleType>::type const *>(
        mRemovedStore.template get<Index>());
}

template <typename IdType, typename... Components>
template <int Index>
auto foeDataPool<IdType, Components...>::crmbegin() const noexcept ->
    typename std::tuple_element<Index, TupleType>::type const * {
    return static_cast<typename std::tuple_element<Index, TupleType>::type const *>(
        mRemovedStore.template get<Index>());
}

template <typename IdType, typename... Components>
template <int Index>
auto foeDataPool<IdType, Components...>::rmend() noexcept ->
    typename std::tuple_element<Index, TupleType>::type * {
    return static_cast<typename std::tuple_element<Index, TupleType>::type *>(
        mRemovedStore.template get<Index>() + mRemoved);
}

template <typename IdType, typename... Components>
template <int Index>
auto foeDataPool<IdType, Components...>::rmend() const noexcept ->
    typename std::tuple_element<Index, TupleType>::type const * {
    return static_cast<typename std::tuple_element<Index, TupleType>::type const *>(
        mRemovedStore.template get<Index>() + mRemoved);
}

template <typename IdType, typename... Components>
template <int Index>
auto foeDataPool<IdType, Components...>::crmend() const noexcept ->
    typename std::tuple_element<Index, TupleType>::type const * {
    return static_cast<typename std::tuple_element<Index, TupleType>::type const *>(
        mRemovedStore.template get<Index>() + mRemoved);
}

template <typename IdType, typename... Components>
template <int Index>
void foeDataPool<IdType, Components...>::singleEmplace(PoolStore *pStore,
                                                       size_t offset,
                                                       IdType id,
                                                       std::tuple<Components...> &&components) {
    auto *pData = pStore->template get<Index>() + offset;

    if constexpr (Index == 0) {
        // Specifically for the ID
        *pData = id;
    } else {
        // Components
        using IndexType = typename std::tuple_element<Index, TupleType>::type;

        new (pData) IndexType{std::move(std::get<Index - 1>(components))};
    }

    if constexpr (Index != sizeof...(Components)) {
        singleEmplace<Index + 1>(pStore, offset, id, std::move(components));
    }
}

template <typename IdType, typename... Components>
template <int Index>
void foeDataPool<IdType, Components...>::multiMove(
    PoolStore *pSrc, size_t srcOffset, size_t count, PoolStore *pDst, size_t dstOffset) {
    using IndexType = typename std::tuple_element<Index, TupleType>::type;

    auto *pData = pSrc->template get<Index>() + srcOffset;
    auto *pDstData = pDst->template get<Index>() + dstOffset;

    if constexpr (std::is_trivially_copyable<IndexType>::value) {
        memmove(pDstData, pData, count * sizeof(IdType));
    } else {
        // Components
        auto *pDataEnd = pData + count;

        while (pData != pDataEnd) {
            new (pDstData++) IndexType{std::move(*pData++)};
        }
    }

    // If not the last component type, go deeper
    if constexpr (Index != sizeof...(Components)) {
        multiMove<Index + 1>(pSrc, srcOffset, count, pDst, dstOffset);
    }
}

template <typename IdType, typename... Components>
template <int Index>
void foeDataPool<IdType, Components...>::multiDelete(PoolStore *pStore,
                                                     size_t offset,
                                                     size_t count) {
    using IndexType = typename std::tuple_element<Index, TupleType>::type;

    if constexpr (!std::is_trivially_destructible<IndexType>::value) {
        auto *pData = pStore->template get<Index>() + offset;
        auto *const pDataEnd = pData + count;

        while (pData != pDataEnd) {
            (pData++)->~IndexType();
        }
    }

    // If not the last component type, go deeper
    if constexpr (Index != sizeof...(Components)) {
        multiDelete<Index + 1>(pStore, offset, count);
    }
}

template <typename IdType, typename... Components>
void foeDataPool<IdType, Components...>::clearInserted() {
    mInsertedOffsets.clear();
}

template <typename IdType, typename... Components>
void foeDataPool<IdType, Components...>::insertPass() {
    mToInsertSync.lock();
    auto toInsert = std::move(mToInsert);
    mToInsertSync.unlock();

    // If there's nothing to insert, leave
    if (toInsert.empty())
        return;

    std::sort(toInsert.begin(), toInsert.end(),
              [](auto const &a, auto const &b) { return std::get<0>(a) < std::get<0>(b); });
    toInsert.erase(foe::unique_last(toInsert.begin(), toInsert.end(),
                                    [](auto const &a, auto const &b) {
                                        return std::get<0>(a) == std::get<0>(b);
                                    }),
                   toInsert.end());

    mInsertedOffsets.reserve(toInsert.size());

    std::vector<std::pair<
        typename std::vector<std::tuple<IdType, std::tuple<Components...>>>::iterator, size_t>>
        srcDstOffsets;
    srcDstOffsets.reserve(toInsert.size());

    {
        // Iterators to the insertion map items
        auto inIt = toInsert.begin();
        auto const inEndIt = toInsert.end();
        // Pointer/Iterator to the IdType's of items already in regular storage
        auto *const pBeginId = mMainStorage.get();
        auto *const pEndId = mMainStorage.get() + mStored;
        auto *pId = pBeginId;

        while (inIt != inEndIt) {
            // Try to shortcut the linear search by doing binary search
            pId = std::lower_bound(pId, pEndId, std::get<0>(*inIt));

            // Find the next destination placement
            while (pId != pEndId && *pId < std::get<0>(*inIt)) {
                ++pId;
            }

            // Same values, item already in regular list, skip the insertion of this element
            if (pId != pEndId && *pId == std::get<0>(*inIt)) {
                ++inIt;
                continue;
            }

            // At this point, it is not already in storage, and is unique, so it is to be added
            srcDstOffsets.emplace_back(inIt, pId - pBeginId);

            ++inIt;
        }
    }

    // If nothing to be inserted, just skip out
    if (srcDstOffsets.empty()) {
        return;
    }

    // Resize main storage if necessary
    PoolStore possibleNewStore;
    // By default new storage is the same storage
    PoolStore *pNewStore = &mMainStorage;
    // If we need a newly resized storage, change where the above pointer looks to
    if (auto newMinSize = mStored + srcDstOffsets.size(); mMainStorage.capacity() < newMinSize) {
        newMinSize = ((newMinSize / mExpansionRate) + 1) * mExpansionRate;
        possibleNewStore = std::move(PoolStore{newMinSize});
        pNewStore = &possibleNewStore;
    }

    size_t lastMovedOldItem = mStored;
    size_t accumulatedDistance = srcDstOffsets.size();

    for (auto srcDstIt = srcDstOffsets.rbegin(); srcDstIt != srcDstOffsets.rend(); ++srcDstIt) {
        size_t givenOffset = srcDstIt->second;
        srcDstIt->second += accumulatedDistance - 1;

        // If the item is being placed before some old objects, then we need to shift them up
        if (givenOffset < lastMovedOldItem) {
            size_t src = givenOffset;
            size_t dst = givenOffset + accumulatedDistance;
            size_t numMove = lastMovedOldItem - givenOffset;

            // @todo Revisit idea of doing 'floating memory' as this is by far the worst time sink
            // during execution
            multiMove(&mMainStorage, src, numMove, pNewStore, dst);

            lastMovedOldItem = givenOffset;
        }

        // Everything has been shifted out of the way, place the entry now
        singleEmplace(pNewStore, srcDstIt->second, std::get<0>(*srcDstIt->first),
                      std::move(std::get<1>(*srcDstIt->first)));

        // We added an item, decrement the accumulated distance
        --accumulatedDistance;

        // Add the aded ID to the inserted list
        mInsertedOffsets.emplace_back(srcDstIt->second);
    }

    // If there is a new storage medium, then change over
    if (pNewStore != &mMainStorage) {
        // Make sure, if we are changing storage, that the last items are moved over.
        if (0 < lastMovedOldItem) {
            size_t src = 0;
            size_t dst = accumulatedDistance;
            size_t numMove = lastMovedOldItem;
            multiMove(&mMainStorage, src, numMove, pNewStore, dst);
        }

        mMainStorage = std::move(possibleNewStore);
    }

    // Set the new number of held items
    mStored += srcDstOffsets.size();

    // Reverse the inserted lists to be in ascending order
    std::reverse(mInsertedOffsets.begin(), mInsertedOffsets.end());
}

template <typename IdType, typename... Components>
void foeDataPool<IdType, Components...>::clearRemoved() {
    multiDelete(&mRemovedStore, 0, mRemoved);
    mRemoved = 0;
}

template <typename IdType, typename... Components>
void foeDataPool<IdType, Components...>::removePass() {
    mToRemoveSync.lock();
    auto toRemove = std::move(mToRemove);
    mToRemoveSync.unlock();

    // If there's nothing to remove, leave
    if (toRemove.empty())
        return;

    // Sort the IDs to be in-order
    std::sort(toRemove.begin(), toRemove.end());

    // Prepare removed storage for new items, make sure has enough capacity
    if (mRemovedStore.capacity() < toRemove.size()) {
        mRemovedStore = std::move(PoolStore{toRemove.size()});
    }

    auto *const pIdStart = mMainStorage.template get<0>();
    auto *const pIdEnd = pIdStart + mStored;
    auto *pId = pIdStart;

    auto rmIt = toRemove.cbegin();
    auto const rmItEnd = toRemove.cend();

    size_t lastMovedObject = 0;
    while (pId != pIdEnd && rmIt != rmItEnd) {
        // If we haven't found the next item to remove yet, continue the search
        if (*pId < *rmIt) {
            ++pId;
            continue;
        }

        if (*pId == *rmIt) {
            // Move the removed entry out
            auto const offset = pId - pIdStart;
            multiMove(&mMainStorage, offset, 1, &mRemovedStore, mRemoved);

            // Move any regular object that need to be shifted down.
            if (lastMovedObject != 0) {
                multiMove(&mMainStorage, lastMovedObject, offset - lastMovedObject, &mMainStorage,
                          lastMovedObject - mRemoved);
            }

            ++pId;
            ++rmIt;
            // We removed another and it's used in-loop, increment
            ++mRemoved;
            lastMovedObject = offset + 1;
        } else {
            // The ID is larger than the ID to remove, so the ID isn't in the pool, so skip this
            // remove request
            ++rmIt;
        }
    }

    if (lastMovedObject != 0) {
        multiMove(&mMainStorage, lastMovedObject, mStored - lastMovedObject, &mMainStorage,
                  lastMovedObject - mRemoved);
    }

    // Finally, decrement the total stored by the number we just removed.
    mStored -= mRemoved;
}

#endif // FOE_ECS_DATA_POOL_IMPL_HPP