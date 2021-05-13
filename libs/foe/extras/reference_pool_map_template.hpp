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

#ifndef STATE_POOL_TEMPLATE_MAP_HPP
#define STATE_POOL_TEMPLATE_MAP_HPP

#include <map>
#include <mutex>
#include <shared_mutex>
#include <vector>

template <typename IdType, typename StateType>
class StateDataMapPool {
    /// Common
  public:
  private:
    std::shared_mutex mReadWriteSync;

    /// Regular
  public:
    void maintenance() {
        clearRemoved();
        clearInserted();

        removePass();
        insertPass();
    }

    size_t size() { return mMainStorage.size(); }

    StateType *get(IdType id) {
        auto searchIt = mMainStorage.find(id);
        if (searchIt != mMainStorage.end()) {
            return &searchIt->second;
        }
        return nullptr;
    }

    auto begin() noexcept { return mMainStorage.begin(); }
    auto cbegin() const noexcept { return mMainStorage.cbegin(); }

    auto end() noexcept { return mMainStorage.end(); }
    auto cend() noexcept { return mMainStorage.cend(); }

  private:
    std::map<IdType, StateType> mMainStorage;

    /// Insertion
  public:
    void insert(IdType id, StateType &&state) {
        mToInsertSync.lock();

        mToInsert.emplace(id, std::move(state));

        mToInsertSync.unlock();
    }

    size_t inserted() { return mInserted.size(); };

    std::vector<IdType> getInserted() { return mInserted; }

    auto inbegin() noexcept { return mInsertedIters.begin(); }
    auto cinbegin() const noexcept { return mInsertedIters.cbegin(); }

    auto inend() noexcept { return mInsertedIters.end(); }
    auto cinend() const noexcept { return mInsertedIters.cend(); }

  private:
    void clearInserted() {
        mInserted.clear();
        mInsertedIters.clear();
    }
    void insertPass();

    std::mutex mToInsertSync;
    std::map<IdType, StateType> mToInsert;

    std::vector<IdType> mInserted;
    std::vector<typename std::map<IdType, StateType>::iterator> mInsertedIters;

    /// Removal
  public:
    void remove(IdType id) {
        mToRemoveSync.lock();
        mToRemove.emplace_back(id);
        mToRemoveSync.unlock();
    }

    size_t removed() { return mRemoved.size(); }

    auto rmbegin() noexcept { return mRemoved.begin(); }
    auto crmbegin() const noexcept { return mRemoved.cbegin(); }

    auto rmend() noexcept { return mRemoved.end(); }
    auto crmend() noexcept { return mRemoved.cend(); }

  private:
    void clearRemoved() { mRemoved.clear(); }
    void removePass();

    // To be removed stuff
    std::mutex mToRemoveSync;
    std::vector<IdType> mToRemove;

    // Stuff removed last maintenance cycle
    std::map<IdType, StateType> mRemoved;
};

template <typename IdType, typename StateType>
void StateDataMapPool<IdType, StateType>::insertPass() {
    mToInsertSync.lock();
    auto toInsert = std::move(mToInsert);
    mToInsertSync.unlock();

    // If there's nothing to insert, leave
    if (toInsert.empty())
        return;

    mInserted.reserve(toInsert.size());
    mInsertedIters.reserve(toInsert.size());

    for (auto &it : toInsert) {
        auto searchIt = mMainStorage.find(it.first);
        if (searchIt != mMainStorage.end()) {
            // It already exists in the main storage, don't overwrite
            continue;
        }

        mMainStorage.emplace(it.first, std::move(it.second));
        mInserted.emplace_back(it.first);
        mInsertedIters.push_back(mMainStorage.find(it.first));
    }
}

template <typename IdType, typename StateType>
void StateDataMapPool<IdType, StateType>::removePass() {
    mToRemoveSync.lock();
    auto toRemove = std::move(mToRemove);
    mToRemoveSync.unlock();

    // If there's nothing to remove, leave
    if (toRemove.empty())
        return;

    // Sort the IDs to be in-order
    std::sort(toRemove.begin(), toRemove.end());

    // Prepare removed storage for new items, make sure has enough capacity
    // mRemovedStorage.reserve(toRemove.size());

    for (auto const &it : toRemove) {
        auto searchIt = mMainStorage.find(it);
        if (searchIt == mMainStorage.end()) {
            // It's not in the main storage
            continue;
        }

        mRemoved.emplace(it, std::move(searchIt->second));
        mMainStorage.erase(searchIt);
    }
}

#endif // STATE_POOL_TEMPLATE_MAP_HPP