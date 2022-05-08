/*
    Copyright (C) 2022 George Cave.

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

#include <foe/resource/pool.h>

#include <foe/ecs/id_to_string.hpp>
#include <foe/resource/resource_fns.h>

#include "error_code.hpp"
#include "log.hpp"

#include <mutex>
#include <shared_mutex>
#include <vector>

namespace {

struct ResourcePool {
    foeResourcePoolType resourcePoolType;
    foeResourceType resourceType;
    size_t resourceSize;

    std::shared_mutex sync;
    foeResourceFns callbacks;
    std::vector<foeResource> resources;
};

FOE_DEFINE_HANDLE_CASTS(resource_pool, ResourcePool, foeResourcePool)

} // namespace

extern "C" foeErrorCode foeCreateResourcePool(foeResourceFns const *pResourceFns,
                                              foeResourcePoolType resourcePoolType,
                                              foeResourceType resourceType,
                                              size_t resourceSize,
                                              foeResourcePool *pResourcePool) {
    ResourcePool *pNewResourcePool = new ResourcePool{
        .resourcePoolType = resourcePoolType,
        .resourceType = resourceType,
        .resourceSize = resourceSize,
        .callbacks = *pResourceFns,
    };

    *pResourcePool = resource_pool_to_handle(pNewResourcePool);

    return foeToErrorCode(FOE_RESOURCE_SUCCESS);
}

extern "C" void foeDestroyResourcePool(foeResourcePool resourcePool) {
    ResourcePool *pResourcePool = resource_pool_from_handle(resourcePool);

    for (auto const resource : pResourcePool->resources) {
        int refCount = foeResourceDecrementRefCount(resource);

        if (refCount == 0) {
            foeDestroyResource(resource);
        } else {
            FOE_LOG(foeResourceCore, Warning,
                    "[{},{}] foeResourcePool - While destroying, found foeResource [{},{}] that "
                    "still has external references and thus skipped immediate destruction",
                    (void *)pResourcePool, pResourcePool->resourcePoolType,
                    foeIdToString(foeResourceGetID(resource)), foeResourceGetType(resource));
        }
    }

    delete pResourcePool;
}

extern "C" foeResource foeResourcePoolAdd(foeResourcePool resourcePool, foeResourceID resourceID) {
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
    std::error_code errC =
        foeCreateResource(resourceID, pResourcePool->resourceType, &pResourcePool->callbacks,
                          pResourcePool->resourceSize, &newResource);
    if (errC)
        return FOE_NULL_HANDLE;

    foeResourceIncrementRefCount(newResource);

    pResourcePool->resources.emplace_back(newResource);

    return newResource;
}

extern "C" foeResource foeResourcePoolFind(foeResourcePool resourcePool, foeResourceID resourceID) {
    ResourcePool *pResourcePool = resource_pool_from_handle(resourcePool);

    std::shared_lock lock{pResourcePool->sync};

    for (auto const it : pResourcePool->resources) {
        if (foeResourceGetID(it) == resourceID) {
            return it;
            break;
        }
    }

    return FOE_NULL_HANDLE;
}

extern "C" foeErrorCode foeResourcePoolRemove(foeResourcePool resourcePool,
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
        return foeToErrorCode(FOE_RESOURCE_ERROR_NOT_FOUND);

    int refCount = foeResourceDecrementRefCount(resource);
    if (refCount == 0) {
        foeDestroyResource(resource);
    } else {
        FOE_LOG(foeResourceCore, Warning,
                "[{},{}] foeResourcePool - While removing foeResource [{},{}], it still has "
                "external references and thus skipped immediate destruction",
                (void *)pResourcePool, pResourcePool->resourcePoolType,
                foeIdToString(foeResourceGetID(resource)), foeResourceGetType(resource));
    }

    return foeToErrorCode(FOE_RESOURCE_SUCCESS);
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
        foeResourceUnload(it, false);
    }
}
