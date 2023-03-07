// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/resource/resource.h>

#include <foe/ecs/id_to_string.hpp>
#include <foe/resource/resource_fns.h>

#include <atomic>
#include <mutex>
#include <thread>

#include "log.hpp"
#include "result.h"

struct foeResourceFns;

namespace {

struct ReplacedResource {
    foeResourceType rType;
    void *pNext;
    foeResource replacementResource;
    bool incrementedUse;
};

struct foeResourceImpl {
    foeResourceID id;

    foeResourceFns const *pResourceFns;

    std::recursive_mutex sync;

    std::atomic_int refCount{1};
    std::atomic_int useCount{0};

    // Load State
    std::atomic_uint iteration{0};
    std::atomic_flag loading = ATOMIC_FLAG_INIT;
    std::atomic<foeResourceLoadState> state{FOE_RESOURCE_LOAD_STATE_UNLOADED};

    // @TODO - Keep loaded ResourceCreateInfo in EditorMode?
    // Perhaps as part of the data next chain

    void *pUnloadContext{nullptr};
    void (*pUnloadFn)(void *, foeResource, uint32_t, PFN_foeResourceUnloadCall, bool){nullptr};

    foeResourceImpl(foeResourceID id, foeResourceFns const *pResourceFns) :
        id{id}, pResourceFns{pResourceFns} {}
};

FOE_DEFINE_HANDLE_CASTS(resource, foeResourceImpl, foeResource)

void postLoadFn(foeResource resource,
                foeResultSet loadResult,
                void *pSrc,
                void (*pMoveDataFn)(void *, void *),
                void *pUnloadContext,
                void (*pUnloadFn)(void *, foeResource, uint32_t, PFN_foeResourceUnloadCall, bool)) {
    auto *pResource = resource_from_handle(resource);

    if (loadResult.value != FOE_SUCCESS) {
        // Loading didn't go well
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        loadResult.toString(loadResult.value, buffer);
        FOE_LOG(foeResource, FOE_LOG_LEVEL_ERROR,
                "[{},{}] foeResource - Failed to load  with error: {}",
                foeIdToString(pResource->id), foeResourceGetType(resource), buffer)

        auto expected = FOE_RESOURCE_LOAD_STATE_UNLOADED;
        pResource->state.compare_exchange_strong(expected, FOE_RESOURCE_LOAD_STATE_FAILED);
    } else {
        pResource->sync.lock();

        // Unload any previous data
        foeResourceUnloadData(resource, true);

        // Move the new data in
        pMoveDataFn(pSrc, (void *)foeResourceGetData(resource));

        pResource->pUnloadContext = pUnloadContext;
        pResource->pUnloadFn = pUnloadFn;

        pResource->state = FOE_RESOURCE_LOAD_STATE_LOADED;
        pResource->sync.unlock();
    }

    pResource->loading.clear();

    // Decrement the reference count of both resource and create info, no longer needed after
    // the loading process is done
    foeResourceDecrementRefCount(resource);
}

void loadResourceTask(foeResourceImpl *pResource) {
    pResource->pResourceFns->pLoadFn(pResource->pResourceFns->pLoadContext,
                                     resource_to_handle(pResource), postLoadFn);
}

} // namespace

extern "C" foeResultSet foeCreateUndefinedResource(foeResourceID id,
                                                   foeResourceFns const *pResourceFns,
                                                   foeResource *pResource) {
    return foeCreateResource(id, FOE_RESOURCE_RESOURCE_TYPE_UNDEFINED, pResourceFns,
                             sizeof(ReplacedResource), pResource);
}

extern "C" foeResultSet foeCreateResource(foeResourceID id,
                                          foeResourceType type,
                                          foeResourceFns const *pResourceFns,
                                          size_t size,
                                          foeResource *pResource) {
    if (pResourceFns == nullptr)
        return to_foeResult(FOE_RESOURCE_ERROR_RESOURCE_FUNCTIONS_NOT_PROVIDED);
    if (size < sizeof(foeResourceBase))
        return to_foeResult(FOE_RESOURCE_ERROR_DATA_SIZE_SMALLER_THAN_BASE);

    auto *pNewResource = (foeResourceImpl *)malloc(sizeof(foeResourceImpl) + size);
    if (pNewResource == NULL)
        return to_foeResult(FOE_RESOURCE_ERROR_OUT_OF_MEMORY);

    new (pNewResource) foeResourceImpl(id, pResourceFns);

    // Zero the data area
    void *pData = (void *)foeResourceGetData(resource_to_handle(pNewResource));
    memset(pData, 0, size);

    // Set the initial type
    foeResourceBase *pBaseData =
        (foeResourceBase *)foeResourceGetData(resource_to_handle(pNewResource));
    pBaseData->rType = type;

    *pResource = resource_to_handle(pNewResource);

    FOE_LOG(foeResource, FOE_LOG_LEVEL_VERBOSE, "[{},{}] foeResource - Created @ {}",
            foeIdToString(id), type, (void *)pNewResource)

    return to_foeResult(FOE_RESOURCE_SUCCESS);
}

extern "C" foeResultSet foeCreateLoadedResource(
    foeResourceID id,
    foeResourceType type,
    foeResourceFns const *pResourceFns,
    size_t size,
    void *pSrc,
    void (*pMoveFn)(void *, void *),
    void *pUnloadContext,
    void (*pUnloadFn)(void *, foeResource, uint32_t, PFN_foeResourceUnloadCall, bool),
    foeResource *pResource) {
    foeResultSet result = foeCreateResource(id, type, pResourceFns, size, pResource);

    if (result.value == FOE_SUCCESS) {
        // If successfully created, then run the move/load
        foeResourceIncrementRefCount(*pResource);
        postLoadFn(*pResource, result, pSrc, pMoveFn, pUnloadContext, pUnloadFn);
    }

    return result;
}

extern "C" foeResultSet foeResourceReplace(foeResource oldResource, foeResource newResource) {
    foeResourceImpl *pOldResource = resource_from_handle(oldResource);
    foeResultSet result = to_foeResult(FOE_RESOURCE_SUCCESS);

    pOldResource->sync.lock();

    if (foeResourceGetType(oldResource) == FOE_RESOURCE_RESOURCE_TYPE_UNDEFINED) {
        ReplacedResource *pOldData = (ReplacedResource *)foeResourceGetData(oldResource);
        pOldData->replacementResource = newResource;
        foeResourceIncrementRefCount(newResource);

        if (foeResourceGetUseCount(oldResource) != 0) {
            foeResourceIncrementUseCount(newResource);
            pOldData->incrementedUse = true;
        }

        // @TODO - Not sure if this is guarantees therType to happen after the newResource is set,
        // nor that the changes are all made visible to other threads correctly.
        std::atomic_thread_fence(std::memory_order_relaxed);

        pOldData->rType = FOE_RESOURCE_RESOURCE_TYPE_REPLACED;
        pOldResource->state = FOE_RESOURCE_LOAD_STATE_LOADED;
        pOldResource->loading.clear();

        FOE_LOG(foeResource, FOE_LOG_LEVEL_VERBOSE,
                "[{},{}] foeResource - Replacing resource @ {} with type {} @ {}",
                foeIdToString(pOldResource->id), foeResourceGetType(oldResource),
                (void *)pOldResource, foeResourceGetType(newResource), (void *)newResource);
    } else {
        result = to_foeResult(FOE_RESOURCE_ERROR_RESOURCE_NOT_UNDEFINED);
    }

    pOldResource->sync.unlock();

    return result;
}

extern "C" foeResource foeResourceGetReplacement(foeResource resource) {
    foeResourceImpl *pResource = resource_from_handle(resource);

    if (foeResourceGetType(resource) == FOE_RESOURCE_RESOURCE_TYPE_REPLACED &&
        foeResourceGetState(resource) == FOE_RESOURCE_LOAD_STATE_LOADED) {
        ReplacedResource *pData = (ReplacedResource *)foeResourceGetData(resource);

        foeResourceIncrementRefCount(pData->replacementResource);
        return pData->replacementResource;
    }

    return FOE_NULL_HANDLE;
}
extern "C" foeResourceID foeResourceGetID(foeResource resource) {
    auto *pResource = resource_from_handle(resource);
    return pResource->id;
}

extern "C" foeResourceType foeResourceGetType(foeResource resource) {
    foeResourceBase const *pBaseData = (foeResourceBase *)foeResourceGetData(resource);
    return pBaseData->rType;
}

extern "C" bool foeResourceHasType(foeResource resource, foeResourceType type) {
    return foeResourceGetTypeData(resource, type) != nullptr;
}

extern "C" int foeResourceGetRefCount(foeResource resource) {
    auto *pResource = resource_from_handle(resource);
    return pResource->refCount;
}

extern "C" int foeResourceIncrementRefCount(foeResource resource) {
    auto *pResource = resource_from_handle(resource);
    return ++pResource->refCount;
}

extern "C" int foeResourceDecrementRefCount(foeResource resource) {
    auto *pResource = resource_from_handle(resource);
    int refCount = --pResource->refCount;

    if (refCount == 0) {
        FOE_LOG(foeResource, FOE_LOG_LEVEL_VERBOSE, "[{},{}] foeResource - Destroying",
                foeIdToString(pResource->id), foeResourceGetType(resource))

        int useCount = pResource->useCount;
        if (useCount != 0) {
            FOE_LOG(foeResource, FOE_LOG_LEVEL_WARNING,
                    "[{},{}] foeResource - Destroying with a non-zero use count of: {}",
                    foeIdToString(pResource->id), foeResourceGetType(resource), useCount)
        }

        foeResourceUnloadData(resource, true);

        FOE_LOG(foeResource, FOE_LOG_LEVEL_VERBOSE, "[{},{}] foeResource - Destroyed",
                foeIdToString(pResource->id), foeResourceGetType(resource))

        if (foeResourceGetType(resource) == FOE_RESOURCE_RESOURCE_TYPE_REPLACED) {
            ReplacedResource *pData = (ReplacedResource *)foeResourceGetData(resource);

            if (pData->incrementedUse)
                foeResourceDecrementUseCount(pData->replacementResource);
            foeResourceDecrementRefCount(pData->replacementResource);
        }

        pResource->~foeResourceImpl();

        free(pResource);
    }

    return refCount;
}

extern "C" int foeResourceGetUseCount(foeResource resource) {
    auto *pResource = resource_from_handle(resource);
    return pResource->useCount;
}

extern "C" int foeResourceIncrementUseCount(foeResource resource) {
    auto *pResource = resource_from_handle(resource);
    return ++pResource->useCount;
}

extern "C" int foeResourceDecrementUseCount(foeResource resource) {
    auto *pResource = resource_from_handle(resource);
    int useCount = --pResource->useCount;

    if (useCount == 0 && foeResourceGetType(resource) == FOE_RESOURCE_RESOURCE_TYPE_REPLACED) {
        ReplacedResource *pData = (ReplacedResource *)foeResourceGetData(resource);
        if (pData->incrementedUse) {
            pData->incrementedUse = false;
            foeResourceDecrementUseCount(pData->replacementResource);
        }
    }

    return useCount;
}

extern "C" bool foeResourceGetIsLoading(foeResource resource) {
    auto *pResource = resource_from_handle(resource);
    return pResource->loading.test();
}

extern "C" foeResourceLoadState foeResourceGetState(foeResource resource) {
    auto *pResource = resource_from_handle(resource);
    return pResource->state;
}

extern "C" void const *foeResourceGetData(foeResource resource) {
    auto *pResource = resource_from_handle(resource);
    return (char *)pResource + sizeof(foeResourceImpl);
}

extern "C" void const *foeResourceGetTypeData(foeResource resource, foeResourceType type) {
    foeResourceBase const *pData = (foeResourceBase const *)foeResourceGetData(resource);

    while (pData != nullptr) {
        if (pData->rType == type)
            return pData;

        pData = (foeResourceBase const *)pData->pNext;
    }

    return nullptr;
}

extern "C" foeResultSet foeResourceLoadData(foeResource resource) {
    auto *pResource = resource_from_handle(resource);

    foeResourceType type = foeResourceGetType(resource);
    if (type == FOE_RESOURCE_RESOURCE_TYPE_REPLACED)
        return to_foeResult(FOE_RESOURCE_ERROR_REPLACED_CANNOT_BE_LOADED);

    foeResourceIncrementRefCount(resource);

    // Only want to start loading if the data isn't already slated to be loaded
    if (pResource->loading.test_and_set()) {
        FOE_LOG(foeResource, FOE_LOG_LEVEL_WARNING,
                "[{},{}] foeResource - Resource already loading", foeIdToString(pResource->id),
                foeResourceGetType(resource))
        foeResourceDecrementRefCount(resource);
        return to_foeResult(FOE_RESOURCE_ALREADY_LOADING);
    }

    if (pResource->pResourceFns->scheduleAsyncTask != nullptr) {
        FOE_LOG(foeResource, FOE_LOG_LEVEL_VERBOSE, "[{},{}] foeResource - Loading asynchronously",
                foeIdToString(pResource->id), foeResourceGetType(resource))

        pResource->pResourceFns->scheduleAsyncTask(
            pResource->pResourceFns->pScheduleAsyncTaskContext, (PFN_foeTask)loadResourceTask,
            pResource);
    } else {
        FOE_LOG(foeResource, FOE_LOG_LEVEL_VERBOSE, "[{},{}] foeResource - Loading synchronously",
                foeIdToString(pResource->id), foeResourceGetType(resource))

        loadResourceTask(pResource);
    }

    return to_foeResult(FOE_RESOURCE_SUCCESS);
}

namespace {

bool resourceUnloadCall(foeResource resource,
                        uint32_t iteration,
                        void *pDst,
                        void (*pMoveFn)(void *, void *)) {
    auto *pResource = resource_from_handle(resource);
    bool retVal{false};

    pResource->sync.lock();

    if (iteration == pResource->iteration) {
        retVal = true;

        pMoveFn((void *)foeResourceGetData(resource), pDst);

        pResource->pUnloadContext = nullptr;
        pResource->pUnloadFn = nullptr;
        pResource->state = FOE_RESOURCE_LOAD_STATE_UNLOADED;

        ++pResource->iteration;
    }

    pResource->sync.unlock();

    return retVal;
}

} // namespace

extern "C" void foeResourceUnloadData(foeResource resource, bool immediate) {
    auto *pResource = resource_from_handle(resource);

    pResource->sync.lock();

    if (pResource->pUnloadFn != nullptr) {
        if (int uses = pResource->useCount; uses > 0) {
            FOE_LOG(foeResource, FOE_LOG_LEVEL_WARNING,
                    "[{},{}] foeResource - Unloading while still actively used {} times",
                    foeIdToString(pResource->id), foeResourceGetType(resource), uses)
        }

        if (immediate) {
            FOE_LOG(foeResource, FOE_LOG_LEVEL_VERBOSE,
                    "[{},{}] foeResource - Unloading immediately", foeIdToString(pResource->id),
                    foeResourceGetType(resource))
        } else {
            FOE_LOG(foeResource, FOE_LOG_LEVEL_VERBOSE, "[{},{}] foeResource - Unloading normally",
                    foeIdToString(pResource->id), foeResourceGetType(resource))
        }

        pResource->pUnloadFn(pResource->pUnloadContext, resource, pResource->iteration,
                             resourceUnloadCall, immediate);
    }

    pResource->sync.unlock();
}