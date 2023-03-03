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
    void (*pUnloadFn)(void *, foeResource, uint32_t, PFN_foeResourceUnloadCall *, bool){nullptr};

    foeResourceImpl(foeResourceID id, foeResourceFns const *pResourceFns) :
        id{id}, pResourceFns{pResourceFns} {}
};

FOE_DEFINE_HANDLE_CASTS(resource, foeResourceImpl, foeResource)

} // namespace

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

    // Set the initial type
    foeResourceBase *pBaseData =
        (foeResourceBase *)foeResourceGetData(resource_to_handle(pNewResource));
    pBaseData->rType = type;

    *pResource = resource_to_handle(pNewResource);

    FOE_LOG(foeResource, FOE_LOG_LEVEL_VERBOSE, "[{},{}] foeResource - Created @ {}",
            foeIdToString(id), type, (void *)pNewResource)

    return to_foeResult(FOE_RESOURCE_SUCCESS);
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
    return --pResource->useCount;
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

namespace {

void postLoadFn(
    foeResource resource,
    foeResultSet loadResult,
    void *pSrc,
    void (*pMoveDataFn)(void *, void *),
    void *pUnloadContext,
    void (*pUnloadFn)(void *, foeResource, uint32_t, PFN_foeResourceUnloadCall *, bool)) {
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

    // Decrement the reference count of both resource and create info, no longer needed after the
    // loading process is done
    foeResourceDecrementRefCount(resource);
}

void loadResourceTask(foeResourceImpl *pResource) {
    pResource->pResourceFns->pLoadFn(pResource->pResourceFns->pLoadContext,
                                     resource_to_handle(pResource), postLoadFn);
}

} // namespace

extern "C" void foeResourceLoadData(foeResource resource) {
    auto *pResource = resource_from_handle(resource);

    foeResourceIncrementRefCount(resource);

    // Only want to start loading if the data isn't already slated to be loaded
    if (pResource->loading.test_and_set()) {
        FOE_LOG(foeResource, FOE_LOG_LEVEL_WARNING,
                "[{},{}] foeResource - Resource already loading", foeIdToString(pResource->id),
                foeResourceGetType(resource))
        foeResourceDecrementRefCount(resource);
        return;
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