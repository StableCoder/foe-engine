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

#ifndef FOE_ECS_DATA_POOL_HPP
#define FOE_ECS_DATA_POOL_HPP

#include <foe/ecs/id.hpp>
#include <foe/storage/multi_alloc.hpp>

#include <mutex>
#include <tuple>
#include <vector>

template <typename... Components>
class foeDataPool {
    using TupleType = std::tuple<foeId, Components...>;
    using PoolStore = foeMultiAllocStorage<foeId, Components...>;

  public:
    /**
     * @brief Constructor that can set the starting expansion rate
     * @param expansionRate Rate that the pool will expand by when it enlarges
     */
    foeDataPool(size_t expansionRate = 128);

    /// Data held in the main and removed stores are deleted and memory freed
    ~foeDataPool();

    /**
     * @brief Performs the data remove/insert requested, in that order
     * @warning Any insert requests of data with ID's already in the pool are discarded
     * @note Data can be both removed and inserted in a cycle, since removals occur first
     *
     * The remove pass is performed first, and any requested data is made accessible for from the
     * rmbegin/rmend iterator accessors.
     *
     * Then, any insertions are performed, with any unique ID'd data inserted and made accessible
     * for the inbegin/inend offset iterator accessors.
     */
    void maintenance();

    /// Sets a new rate of data pool expansion
    void expansionRate(size_t expansionRate);
    /// Returns the current rate of pool expansion
    size_t expansionRate() const noexcept;

    /// Returns the current capacity of the pool
    size_t capacity() const noexcept;
    /// Returns the number of elements in the regular pool
    size_t size() const noexcept;
    /// Returns the number of items inserted last maintenance cycle
    size_t inserted();
    /// Returns the number of items removed last maintenance cycle
    size_t removed() const noexcept;

    /// Returns true if data for the ID is already in the pool
    bool exist(foeId id) const noexcept;

    /**
     * @brief Adds associated data with an ID to be inserted next maintenance cycle
     * @param id ID that ties the data together and identifies it uniquely
     * @param components Data being inserted
     * @warning If the data already exists in the pool and isn't removed next cycle, this data
     * *will* be discarded
     */
    void insert(foeId id, Components &&...components);

    /// Adds the given ID for any data present to be removed next maintenance cycle
    void remove(foeId id);

    size_t sequential_search(foeId id) const noexcept;
    size_t binary_search(foeId id) const noexcept;
    size_t find(foeId id) const noexcept;

    template <int Index = 0>
    auto begin() noexcept -> typename std::tuple_element<Index, TupleType>::type *;

    template <int Index = 0>
    auto begin() const noexcept -> typename std::tuple_element<Index, TupleType>::type const *;

    template <int Index = 0>
    auto cbegin() const noexcept -> typename std::tuple_element<Index, TupleType>::type const *;

    template <int Index = 0>
    auto end() noexcept -> typename std::tuple_element<Index, TupleType>::type *;

    template <int Index = 0>
    auto end() const noexcept -> typename std::tuple_element<Index, TupleType>::type const *;

    template <int Index = 0>
    auto cend() const noexcept -> typename std::tuple_element<Index, TupleType>::type const *;

    auto inbegin() noexcept;
    auto inbegin() const noexcept;
    auto cinbegin() const noexcept;

    auto inend() noexcept;
    auto inend() const noexcept;
    auto cinend() const noexcept;

    size_t rm_sequential_search(foeId id) const noexcept;
    size_t rm_binary_search(foeId id) const noexcept;
    size_t rm_find(foeId id) const noexcept;

    template <int Index = 0>
    auto rmbegin() noexcept -> typename std::tuple_element<Index, TupleType>::type *;

    template <int Index = 0>
    auto rmbegin() const noexcept -> typename std::tuple_element<Index, TupleType>::type const *;

    template <int Index = 0>
    auto crmbegin() const noexcept -> typename std::tuple_element<Index, TupleType>::type const *;

    template <int Index = 0>
    auto rmend() noexcept -> typename std::tuple_element<Index, TupleType>::type *;

    template <int Index = 0>
    auto rmend() const noexcept -> typename std::tuple_element<Index, TupleType>::type const *;

    template <int Index = 0>
    auto crmend() const noexcept -> typename std::tuple_element<Index, TupleType>::type const *;

  private:
    /**
     * @brief Emplaces a single ID/dataset at a specified place in a data store
     * @param pStore Data store the data is being moved to
     * @param offset Offset in the data store the item is being emplaced at
     * @param id ID value that ties the data
     * @param components Variadic set of items being moved to the data store
     */
    template <int Index = 0>
    void singleEmplace(PoolStore *pStore,
                       size_t offset,
                       foeId id,
                       std::tuple<Components...> &&components);

    /**
     * @brief Moved stored data objects based on the provided values
     * @param pSrc Data store to move data from
     * @param srcOffset Offset into the store to start moves at
     * @param count Number of items from the offset move
     * @param pDst Data store to move data to
     * @param dstOffset Offset into the destination store to start at
     */
    template <int Index = 0>
    void multiMove(
        PoolStore *pSrc, size_t srcOffset, size_t count, PoolStore *pDst, size_t dstOffset);

    /**
     * @brief Deletes stored data objects based on the provided values
     * @param pStore Data store to delete items from
     * @param offset Offset into the store to start deletions at
     * @param count Number of items from the offset to delete
     */
    template <int Index = 0>
    void multiDelete(PoolStore *pStore, size_t offset, size_t count);

    /// Clears data relating to what was previously inserted
    void clearInserted();
    /// Performs insertion of requested data
    void insertPass();

    /// Clears previously removed data
    void clearRemoved();
    /// Performs removals of requested data
    void removePass();

    /// When the main data store expand, this dictates the multiple of how much
    size_t mExpansionRate;
    /// Currently held IDs and data
    PoolStore mMainStorage;
    /// Number of items in the main store
    size_t mStored;

    /// Synchronizes access to the insertion list
    std::mutex mToInsertSync;
    /// IDs and data to attempt insertion next maintenance
    std::vector<std::tuple<foeId, std::tuple<Components...>>> mToInsert;
    /// List of offsets of items inserted last maintenance
    std::vector<size_t> mInsertedOffsets;

    /// Synchronizes access to the removal list
    std::mutex mToRemoveSync;
    /// List of ID'd items to remove next maintenance
    std::vector<foeId> mToRemove;

    /// Data removed last maintenance
    PoolStore mRemovedStore;
    /// Number of items removed last maintenance
    size_t mRemoved;
};

#include <foe/ecs/data_pool.impl.hpp>

#endif // FOE_ECS_DATA_POOL_HPP