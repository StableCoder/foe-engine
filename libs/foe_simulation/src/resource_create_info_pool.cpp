// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/simulation/resource_create_info_pool.h>

#include "result.h"

#include <algorithm>
#include <mutex>
#include <shared_mutex>
#include <vector>

namespace {

struct Entry {
    foeResourceID id;
    foeResourceCreateInfo createInfo;
};

struct ResourceCreateInfoPool {
    std::shared_mutex sync;
    std::vector<Entry> entries;
};

FOE_DEFINE_HANDLE_CASTS(create_info_pool, ResourceCreateInfoPool, foeResourceCreateInfoPool)

} // namespace

extern "C" foeResultSet foeCreateResourceCreateInfoPool(
    foeResourceCreateInfoPool *pResourceCreateInfoPool) {
    ResourceCreateInfoPool *pNewCreateInfoPool = new (std::nothrow) ResourceCreateInfoPool;
    if (pNewCreateInfoPool == nullptr)
        return to_foeResult(FOE_SIMULATION_ERROR_OUT_OF_MEMORY);

    *pResourceCreateInfoPool = create_info_pool_to_handle(pNewCreateInfoPool);

    return to_foeResult(FOE_SIMULATION_SUCCESS);
}

extern "C" void foeDestroyResourceCreateInfoPool(foeResourceCreateInfoPool resourceCreateInfoPool) {
    ResourceCreateInfoPool *pCreateInfoPool = create_info_pool_from_handle(resourceCreateInfoPool);

    for (Entry const &it : pCreateInfoPool->entries) {
        foeResourceCreateInfoDecrementRefCount(it.createInfo);
        // @TODO - Log non-zero counts
    }

    delete pCreateInfoPool;
}

extern "C" foeResultSet foeResourceCreateInfoPoolAdd(
    foeResourceCreateInfoPool resourceCreateInfoPool,
    foeResourceID resourceID,
    foeResourceCreateInfo resourceCreateInfo) {
    ResourceCreateInfoPool *pCreateInfoPool = create_info_pool_from_handle(resourceCreateInfoPool);

    std::unique_lock lock{pCreateInfoPool->sync};

    auto searchIt = std::lower_bound(
        pCreateInfoPool->entries.begin(), pCreateInfoPool->entries.end(), resourceID,
        [](Entry const &entry, foeResourceID resourceID) { return entry.id < resourceID; });

    if (searchIt != pCreateInfoPool->entries.end() && searchIt->id == resourceID)
        return to_foeResult(FOE_SIMULATION_ERROR_RESOURCE_CREATE_INFO_ALREADY_ADDED);

    pCreateInfoPool->entries.emplace(searchIt, Entry{
                                                   .id = resourceID,
                                                   .createInfo = resourceCreateInfo,
                                               });

    lock.unlock();

    foeResourceCreateInfoIncrementRefCount(resourceCreateInfo);

    return to_foeResult(FOE_SIMULATION_SUCCESS);
}

extern "C" foeResultSet foeResourceCreateInfoPoolRemove(
    foeResourceCreateInfoPool resourceCreateInfoPool, foeResourceID resourceID) {
    ResourceCreateInfoPool *pCreateInfoPool = create_info_pool_from_handle(resourceCreateInfoPool);

    std::unique_lock lock{pCreateInfoPool->sync};

    auto searchIt = std::lower_bound(
        pCreateInfoPool->entries.begin(), pCreateInfoPool->entries.end(), resourceID,
        [](Entry const &entry, foeResourceID resourceID) { return entry.id < resourceID; });

    if (searchIt == pCreateInfoPool->entries.end() || searchIt->id != resourceID)
        return to_foeResult(FOE_SIMULATION_ERROR_RESOURCE_CREATE_INFO_NOT_FOUND);

    foeResourceCreateInfo createInfo = searchIt->createInfo;
    pCreateInfoPool->entries.erase(searchIt);

    lock.unlock();

    // @TODO - Log non-zero counts
    foeResourceCreateInfoDecrementRefCount(createInfo);

    return to_foeResult(FOE_SIMULATION_SUCCESS);
}

extern "C" foeResourceCreateInfo foeResourceCreateInfoPoolGet(
    foeResourceCreateInfoPool resourceCreateInfoPool, foeResourceID resourceID) {
    ResourceCreateInfoPool *pCreateInfoPool = create_info_pool_from_handle(resourceCreateInfoPool);

    std::shared_lock lock{pCreateInfoPool->sync};

    auto searchIt = std::lower_bound(
        pCreateInfoPool->entries.begin(), pCreateInfoPool->entries.end(), resourceID,
        [](Entry const &entry, foeResourceID resourceID) { return entry.id < resourceID; });

    if (searchIt == pCreateInfoPool->entries.end() || searchIt->id != resourceID)
        return FOE_NULL_HANDLE;

    foeResourceCreateInfo createInfo = searchIt->createInfo;

    lock.unlock();

    foeResourceCreateInfoIncrementRefCount(createInfo);

    return createInfo;
}