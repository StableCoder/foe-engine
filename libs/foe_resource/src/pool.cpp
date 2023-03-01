// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/resource/pool.h>

#include <foe/ecs/id_to_string.hpp>
#include <foe/resource/resource_fns.h>

#include "log.hpp"
#include "result.h"

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

extern "C" foeResource foeResourcePoolAdd(foeResourcePool resourcePool,
                                          foeResourceID resourceID,
                                          foeResourceType resourceType,
                                          size_t resourceSize) {
    ResourcePool *pResourcePool = resource_pool_from_handle(resourcePool);

    std::unique_lock lock{pResourcePool->sync};

    // If it finds it, return nullptr
    for (auto const it : pResourcePool->resources) {
        if (foeResourceGetID(it) == resourceID) {
            return FOE_NULL_HANDLE;
        }
    }

    // Not found, add it
    foeResource newResource;
    foeResultSet result = foeCreateResource(resourceID, resourceType, &pResourcePool->callbacks,
                                            resourceSize, &newResource);
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

    pResourcePool->resources.emplace_back(newResource);
    return newResource;
}

extern "C" foeResource foeResourcePoolFind(foeResourcePool resourcePool, foeResourceID resourceID) {
    ResourcePool *pResourcePool = resource_pool_from_handle(resourcePool);

    std::shared_lock lock{pResourcePool->sync};

    for (auto const it : pResourcePool->resources) {
        if (foeResourceGetID(it) == resourceID) {
            // Since we're returning the resource, increment the count to account for that
            foeResourceIncrementRefCount(it);

            return it;
        }
    }

    return FOE_NULL_HANDLE;
}

extern "C" foeResultSet foeResourcePoolRemove(foeResourcePool resourcePool,
                                              foeResourceID resourceID) {
    ResourcePool *pResourcePool = resource_pool_from_handle(resourcePool);

    std::unique_lock lock{pResourcePool->sync};
    foeResource resource{FOE_NULL_HANDLE};

    for (auto it = pResourcePool->resources.begin(); it != pResourcePool->resources.end(); ++it) {
        if (foeResourceGetID(*it) == resourceID) {
            resource = *it;
            pResourcePool->resources.erase(it);
        }
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

extern "C" void foeResourcePoolAddAsyncTaskCallback(foeResourcePool resourcePool,
                                                    PFN_foeScheduleTask scheduleAsyncTask,
                                                    void *pScheduleAsyncTaskContext) {
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
