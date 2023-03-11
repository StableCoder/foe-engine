// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/resource/pool.h>

#include <foe/ecs/id_to_string.hpp>
#include <foe/resource/resource_fns.h>

#include "log.hpp"
#include "result.h"

#include <algorithm>
#include <mutex>
#include <shared_mutex>
#include <vector>

namespace {

struct ResourcePool {
    std::shared_mutex sync;
    foeResourceFns callbacks;
    std::vector<foeResource> resources;
};

FOE_DEFINE_HANDLE_CASTS(resource_pool, ResourcePool, foeResourcePool)

} // namespace

extern "C" foeResultSet foeCreateResourcePool(foeResourceFns const *pResourceFns,
                                              foeResourcePool *pResourcePool) {
    ResourcePool *pNewResourcePool = new (std::nothrow) ResourcePool{
        .callbacks = *pResourceFns,
    };
    if (pNewResourcePool == NULL)
        return to_foeResult(FOE_RESOURCE_ERROR_OUT_OF_MEMORY);

    *pResourcePool = resource_pool_to_handle(pNewResourcePool);

    return to_foeResult(FOE_RESOURCE_SUCCESS);
}

extern "C" void foeDestroyResourcePool(foeResourcePool resourcePool) {
    ResourcePool *pResourcePool = resource_pool_from_handle(resourcePool);

    for (auto const resource : pResourcePool->resources) {
        int refCount = foeResourceDecrementRefCount(resource);

        if (refCount != 0) {
            FOE_LOG(foeResource, FOE_LOG_LEVEL_WARNING,
                    "[{}] foeResourcePool - While destroying, found foeResource [{},{}] that "
                    "still has external references and thus skipped immediate destruction",
                    (void *)pResourcePool, foeIdToString(foeResourceGetID(resource)),
                    foeResourceGetType(resource));
        }
    }

    delete pResourcePool;
}

extern "C" foeResource foeResourcePoolAdd(foeResourcePool resourcePool, foeResourceID resourceID) {
    ResourcePool *pResourcePool = resource_pool_from_handle(resourcePool);

    std::unique_lock lock{pResourcePool->sync};

    auto searchIt = std::lower_bound(
        pResourcePool->resources.begin(), pResourcePool->resources.end(), resourceID,
        [](foeResource resource, foeResourceID id) { return foeResourceGetID(resource) < id; });

    if (searchIt != pResourcePool->resources.end() && foeResourceGetID(*searchIt) == resourceID)
        // Didn't find the resource we're supposed to be replacing
        return FOE_NULL_HANDLE;

    // Not found, add it
    foeResource newResource;
    foeResultSet result =
        foeCreateUndefinedResource(resourceID, &pResourcePool->callbacks, &newResource);
    if (result.value != FOE_RESOURCE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeResource, FOE_LOG_LEVEL_ERROR,
                "[{}] foeResourcePool - Error while creating a new resource [{}]: {}",
                (void *)pResourcePool, foeIdToString(resourceID), buffer)

        return FOE_NULL_HANDLE;
    }

    // Since we're returning the resource, increment the count to account for that
    foeResourceIncrementRefCount(newResource);

    pResourcePool->resources.insert(searchIt, newResource);
    return newResource;
}

extern "C" foeResource foeResourcePoolLoadedReplace(
    foeResourcePool resourcePool,
    foeResourceID resourceID,
    foeResourceType resourceType,
    size_t resourceSize,
    void *pLoadDataContext,
    PFN_foeResourceDataModify loadDataFn,
    void *pUnloadDataContext,
    void (*pUnloadDataFn)(void *, foeResource, uint32_t, PFN_foeResourceUnloadCall, bool)) {
    ResourcePool *pResourcePool = resource_pool_from_handle(resourcePool);

    std::unique_lock lock{pResourcePool->sync};

    auto searchIt = std::lower_bound(
        pResourcePool->resources.begin(), pResourcePool->resources.end(), resourceID,
        [](foeResource resource, foeResourceID id) { return foeResourceGetID(resource) < id; });

    if (searchIt == pResourcePool->resources.end() || foeResourceGetID(*searchIt) != resourceID)
        // Didn't find the resource we're supposed to be replacing
        return FOE_NULL_HANDLE;

    // Creat the new resource that will be the replacement
    foeResource newResource = FOE_NULL_HANDLE;
    foeResultSet result = foeCreateLoadedResource(
        resourceID, resourceType, &pResourcePool->callbacks, resourceSize, pLoadDataContext,
        loadDataFn, pUnloadDataContext, pUnloadDataFn, &newResource);
    if (result.value != FOE_RESOURCE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeResource, FOE_LOG_LEVEL_ERROR,
                "[{}] foeResourcePool - Error while creating a new replacement resource [{}]: {}",
                (void *)pResourcePool, foeIdToString(resourceID), buffer)

        return FOE_NULL_HANDLE;
    }

    result = foeResourceReplace(*searchIt, newResource);
    if (result.value != FOE_RESOURCE_SUCCESS) {
        foeResourceDecrementRefCount(newResource);
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeResource, FOE_LOG_LEVEL_ERROR,
                "[{}] foeResourcePool - Error while replacing a resource [{}]: {}",
                (void *)pResourcePool, foeIdToString(resourceID), buffer)

        return FOE_NULL_HANDLE;
    }

    // Swap the resources
    foeResource oldResource = *searchIt;
    *searchIt = newResource;

    lock.unlock();

    // Decrement use of old resource
    foeResourceDecrementRefCount(oldResource);
    // Increment new resource before passing it off
    // Should be count of 3:
    // - ref for the pool
    // - ref for replaced
    // - ref being passed back
    foeResourceIncrementRefCount(newResource);

    return newResource;
}

extern "C" foeResource foeResourcePoolFind(foeResourcePool resourcePool, foeResourceID resourceID) {
    ResourcePool *pResourcePool = resource_pool_from_handle(resourcePool);

    std::shared_lock lock{pResourcePool->sync};

    auto searchIt = std::lower_bound(
        pResourcePool->resources.begin(), pResourcePool->resources.end(), resourceID,
        [](foeResource resource, foeResourceID id) { return foeResourceGetID(resource) < id; });

    if (searchIt != pResourcePool->resources.end() && foeResourceGetID(*searchIt) == resourceID) {
        foeResourceIncrementRefCount(*searchIt);
        return *searchIt;
    }

    return FOE_NULL_HANDLE;
}

extern "C" foeResultSet foeResourcePoolRemove(foeResourcePool resourcePool,
                                              foeResourceID resourceID) {
    ResourcePool *pResourcePool = resource_pool_from_handle(resourcePool);
    foeResource resource{FOE_NULL_HANDLE};

    std::unique_lock lock{pResourcePool->sync};

    auto searchIt = std::lower_bound(
        pResourcePool->resources.begin(), pResourcePool->resources.end(), resourceID,
        [](foeResource resource, foeResourceID id) { return foeResourceGetID(resource) < id; });

    if (searchIt != pResourcePool->resources.end() && foeResourceGetID(*searchIt) == resourceID) {
        resource = *searchIt;
        pResourcePool->resources.erase(searchIt);
    }
    lock.unlock();

    if (resource == FOE_NULL_HANDLE)
        return to_foeResult(FOE_RESOURCE_ERROR_NOT_FOUND);

    int refCount = foeResourceDecrementRefCount(resource);
    if (refCount != 0) {
        FOE_LOG(foeResource, FOE_LOG_LEVEL_WARNING,
                "[{}] foeResourcePool - While removing foeResource [{},{}], it still has "
                "external references and thus skipped immediate destruction",
                (void *)pResourcePool, foeIdToString(foeResourceGetID(resource)),
                foeResourceGetType(resource));
    }

    return to_foeResult(FOE_RESOURCE_SUCCESS);
}

extern "C" void foeResourcePoolSetAsyncTaskCallback(foeResourcePool resourcePool,
                                                    void *pScheduleAsyncTaskContext,
                                                    PFN_foeScheduleTask scheduleAsyncTask) {
    ResourcePool *pResourcePool = resource_pool_from_handle(resourcePool);

    pResourcePool->callbacks.scheduleAsyncTask = scheduleAsyncTask;
    pResourcePool->callbacks.pScheduleAsyncTaskContext = pScheduleAsyncTaskContext;
}

extern "C" void foeResourcePoolUnloadAll(foeResourcePool resourcePool) {
    ResourcePool *pResourcePool = resource_pool_from_handle(resourcePool);

    std::shared_lock lock{pResourcePool->sync};

    for (foeResource it : pResourcePool->resources) {
        foeResourceUnloadData(it, false);
    }
}

extern "C" uint32_t foeResourcePoolUnloadType(foeResourcePool resourcePool,
                                              foeResourceType resourceType) {
    uint32_t count = 0;
    ResourcePool *pResourcePool = resource_pool_from_handle(resourcePool);

    std::shared_lock lock{pResourcePool->sync};

    for (foeResource it : pResourcePool->resources) {
        if (foeResourceGetType(it) == resourceType &&
            (foeResourceGetIsLoading(it) ||
             foeResourceGetState(it) == FOE_RESOURCE_LOAD_STATE_LOADED)) {
            foeResourceUnloadData(it, false);
            ++count;
        }
    }

    return count;
}
