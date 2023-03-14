// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/simulation/resource_create_info_history.h>

#include "result.h"

#include <algorithm>
#include <mutex>
#include <shared_mutex>
#include <vector>

namespace {

struct Entry {
    foeResourceID id;
    std::vector<foeResourceCreateInfo> history;
    size_t current;
};

struct ResourceCreateInfoHistory {
    std::shared_mutex sync;
    std::vector<Entry> entries;
};

FOE_DEFINE_HANDLE_CASTS(create_info_history,
                        ResourceCreateInfoHistory,
                        foeResourceCreateInfoHistory)

} // namespace

extern "C" foeResultSet foeCreateResourceCreateInfoHistory(
    foeResourceCreateInfoHistory *pResourceCreateInfoHistory) {
    ResourceCreateInfoHistory *pNewCreateInfoHistory = new (std::nothrow) ResourceCreateInfoHistory;
    if (pNewCreateInfoHistory == nullptr)
        return to_foeResult(FOE_SIMULATION_ERROR_OUT_OF_MEMORY);

    *pResourceCreateInfoHistory = create_info_history_to_handle(pNewCreateInfoHistory);

    return to_foeResult(FOE_SIMULATION_SUCCESS);
}

extern "C" void foeDestroyResourceCreateInfoHistory(
    foeResourceCreateInfoHistory resourceCreateInfoHistory) {
    ResourceCreateInfoHistory *pCreateInfoHistory =
        create_info_history_from_handle(resourceCreateInfoHistory);

    for (Entry const &entry : pCreateInfoHistory->entries) {
        for (foeResourceCreateInfo resourceCI : entry.history) {
            foeResourceCreateInfoDecrementRefCount(resourceCI);
            // @TODO - Log non-zero counts
        }
    }

    delete pCreateInfoHistory;
}

extern "C" foeResultSet foeResourceCreateInfoHistoryAdd(
    foeResourceCreateInfoHistory resourceCreateInfoHistory,
    foeResourceID resourceID,
    foeResourceCreateInfo resourceCreateInfo) {
    ResourceCreateInfoHistory *pCreateInfoHistory =
        create_info_history_from_handle(resourceCreateInfoHistory);

    std::unique_lock lock{pCreateInfoHistory->sync};

    auto searchIt = std::lower_bound(
        pCreateInfoHistory->entries.begin(), pCreateInfoHistory->entries.end(), resourceID,
        [](Entry const &entry, foeResourceID resourceID) { return entry.id < resourceID; });

    if (searchIt != pCreateInfoHistory->entries.end() && searchIt->id == resourceID)
        return to_foeResult(FOE_SIMULATION_ERROR_RESOURCE_CREATE_INFO_ALREADY_ADDED);

    pCreateInfoHistory->entries.emplace(searchIt, Entry{
                                                      .id = resourceID,
                                                      .history = {resourceCreateInfo},
                                                      .current = 0,
                                                  });

    lock.unlock();

    foeResourceCreateInfoIncrementRefCount(resourceCreateInfo);

    return to_foeResult(FOE_SIMULATION_SUCCESS);
}

extern "C" foeResultSet foeResourceCreateInfoHistoryRemove(
    foeResourceCreateInfoHistory resourceCreateInfoHistory, foeResourceID resourceID) {
    ResourceCreateInfoHistory *pCreateInfoHistory =
        create_info_history_from_handle(resourceCreateInfoHistory);

    std::unique_lock lock{pCreateInfoHistory->sync};

    auto searchIt = std::lower_bound(
        pCreateInfoHistory->entries.begin(), pCreateInfoHistory->entries.end(), resourceID,
        [](Entry const &entry, foeResourceID resourceID) { return entry.id < resourceID; });

    if (searchIt == pCreateInfoHistory->entries.end() || searchIt->id != resourceID)
        return to_foeResult(FOE_SIMULATION_ERROR_RESOURCE_CREATE_INFO_NOT_FOUND);

    Entry entry = std::move(*searchIt);
    pCreateInfoHistory->entries.erase(searchIt);

    lock.unlock();

    for (foeResourceCreateInfo resourceCI : entry.history) {
        foeResourceCreateInfoDecrementRefCount(resourceCI);
        // @TODO - Log non-zero counts
    }

    return to_foeResult(FOE_SIMULATION_SUCCESS);
}

extern "C" foeResourceCreateInfo foeResourceCreateInfoHistoryCurrent(
    foeResourceCreateInfoHistory resourceCreateInfoHistory, foeResourceID resourceID) {
    ResourceCreateInfoHistory *pCreateInfoHistory =
        create_info_history_from_handle(resourceCreateInfoHistory);

    std::shared_lock lock{pCreateInfoHistory->sync};

    auto searchIt = std::lower_bound(
        pCreateInfoHistory->entries.begin(), pCreateInfoHistory->entries.end(), resourceID,
        [](Entry const &entry, foeResourceID resourceID) { return entry.id < resourceID; });

    if (searchIt == pCreateInfoHistory->entries.end() || searchIt->id != resourceID)
        return FOE_NULL_HANDLE;

    foeResourceCreateInfo createInfo = searchIt->history[searchIt->current];

    lock.unlock();

    foeResourceCreateInfoIncrementRefCount(createInfo);

    return createInfo;
}

foeResultSet foeResourceCreateInfoHistoryUndo(
    foeResourceCreateInfoHistory resourceCreateInfoHistory, foeResourceID resourceID) {
    ResourceCreateInfoHistory *pCreateInfoHistory =
        create_info_history_from_handle(resourceCreateInfoHistory);

    std::unique_lock lock{pCreateInfoHistory->sync};

    auto searchIt = std::lower_bound(
        pCreateInfoHistory->entries.begin(), pCreateInfoHistory->entries.end(), resourceID,
        [](Entry const &entry, foeResourceID resourceID) { return entry.id < resourceID; });

    if (searchIt == pCreateInfoHistory->entries.end() || searchIt->id != resourceID)
        return to_foeResult(FOE_SIMULATION_ERROR_RESOURCE_CREATE_INFO_NOT_FOUND);

    if (searchIt->current == 0) {
        return to_foeResult(FOE_SIMULATION_CANNOT_UNDO);
    }

    --searchIt->current;

    return to_foeResult(FOE_SIMULATION_SUCCESS);
}

FOE_RES_EXPORT foeResultSet foeResourceCreateInfoHistoryRedo(
    foeResourceCreateInfoHistory resourceCreateInfoHistory, foeResourceID resourceID) {
    ResourceCreateInfoHistory *pCreateInfoHistory =
        create_info_history_from_handle(resourceCreateInfoHistory);

    std::unique_lock lock{pCreateInfoHistory->sync};

    auto searchIt = std::lower_bound(
        pCreateInfoHistory->entries.begin(), pCreateInfoHistory->entries.end(), resourceID,
        [](Entry const &entry, foeResourceID resourceID) { return entry.id < resourceID; });

    if (searchIt == pCreateInfoHistory->entries.end() || searchIt->id != resourceID)
        return to_foeResult(FOE_SIMULATION_ERROR_RESOURCE_CREATE_INFO_NOT_FOUND);

    if (searchIt->current == searchIt->history.size() - 1) {
        return to_foeResult(FOE_SIMULATION_CANNOT_REDO);
    }

    ++searchIt->current;

    return to_foeResult(FOE_SIMULATION_SUCCESS);
}