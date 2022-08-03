// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/resource/resource.h>

#include <foe/ecs/id_to_string.hpp>
#include <foe/resource/resource_fns.h>

#include <atomic>
#include <mutex>

#include "log.hpp"
#include "result.h"

struct foeResourceFns;

namespace {

struct foeResourceImpl {
    foeResourceID id;
    foeResourceType type;

    foeResourceFns const *pResourceFns;

    std::recursive_mutex sync;

    std::atomic_int refCount{0};
    std::atomic_int useCount{0};

    // Create Info State
    foeResourceCreateInfo createInfo{FOE_NULL_HANDLE};

    // Load State
    std::atomic_uint iteration{0};
    std::atomic_bool isLoading{false};
    std::atomic<foeResourceLoadState> state{FOE_RESOURCE_LOAD_STATE_UNLOADED};

    foeResourceCreateInfo loadedCreateInfo{FOE_NULL_HANDLE};
    void *pUnloadContext{nullptr};
    void (*pUnloadFn)(void *, foeResource, uint32_t, PFN_foeResourceUnloadCall *, bool){nullptr};

    foeResourceImpl(foeResourceID id, foeResourceType type, foeResourceFns const *pResourceFns) :
        id{id}, type{type}, pResourceFns{pResourceFns} {}
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

    auto *pNewResource = (foeResourceImpl *)malloc(sizeof(foeResourceImpl) + size);
    if (pNewResource == NULL)
        return to_foeResult(FOE_RESOURCE_ERROR_OUT_OF_MEMORY);

    new (pNewResource) foeResourceImpl(id, type, pResourceFns);

    *pResource = resource_to_handle(pNewResource);

    FOE_LOG(foeResourceCore, Verbose, "[{},{}] foeResource - Created @ {}", foeIdToString(id), type,
            (void *)pNewResource)

    return to_foeResult(FOE_RESOURCE_SUCCESS);
}

extern "C" void foeDestroyResource(foeResource resource) {
    auto *pResource = resource_from_handle(resource);

    FOE_LOG(foeResourceCore, Verbose, "[{},{}] foeResource - Destroying",
            foeIdToString(pResource->id), pResource->type)

    int useCount = pResource->useCount;
    if (useCount != 0) {
        FOE_LOG(foeResourceCore, Warning,
                "[{},{}] foeResource - Destroying with a non-zero use count of: {}",
                foeIdToString(pResource->id), pResource->type, useCount)
    }

    int refCount = pResource->refCount;
    if (refCount != 0) {
        FOE_LOG(foeResourceCore, Warning,
                "[{},{}] foeResource - Destroying with a non-zero reference count of: {}",
                foeIdToString(pResource->id), pResource->type, refCount)
    }

    foeResourceUnload(resource, true);

    // Clear createInfo
    if (pResource->createInfo != FOE_NULL_HANDLE) {
        auto count = foeResourceCreateInfoDecrementRefCount(pResource->createInfo);
        if (count == 0) {
            foeDestroyResourceCreateInfo(pResource->createInfo);
        }
    }

    FOE_LOG(foeResourceCore, Verbose, "[{},{}] foeResource - Destroyed",
            foeIdToString(pResource->id), pResource->type)

    pResource->~foeResourceImpl();

    free(pResource);
}

extern "C" foeResourceID foeResourceGetID(foeResource resource) {
    auto *pResource = resource_from_handle(resource);
    return pResource->id;
}

extern "C" foeResourceType foeResourceGetType(foeResource resource) {
    auto *pResource = resource_from_handle(resource);
    return pResource->type;
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
    return --pResource->refCount;
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
    return pResource->isLoading;
}

extern "C" foeResourceLoadState foeResourceGetState(foeResource resource) {
    auto *pResource = resource_from_handle(resource);
    return pResource->state;
}

extern "C" void const *foeResourceGetData(foeResource resource) {
    auto *pResource = resource_from_handle(resource);
    return (char *)pResource + sizeof(foeResourceImpl);
}

namespace {

void loadCreateInfoTask(foeResourceImpl *pResource) {
    foeResourceCreateInfo newCreateInfo =
        pResource->pResourceFns->pImportFn(pResource->pResourceFns->pImportContext, pResource->id);
    foeResourceCreateInfo oldCreateInfo{FOE_NULL_HANDLE};

    foeResourceCreateInfoIncrementRefCount(newCreateInfo);

    pResource->sync.lock();
    if (newCreateInfo != FOE_NULL_HANDLE) {
        oldCreateInfo = pResource->createInfo;
    }
    pResource->createInfo = newCreateInfo;
    pResource->isLoading = false;
    pResource->sync.unlock();

    foeResourceDecrementRefCount(resource_to_handle(pResource));

    // If destroying old create info data, do it outside the locked area, could be expensive
    if (oldCreateInfo != FOE_NULL_HANDLE) {
        auto refCount = foeResourceCreateInfoDecrementRefCount(oldCreateInfo);
        if (refCount == 0) {
            foeDestroyResourceCreateInfo(oldCreateInfo);
        }
    }
}

} // namespace

extern "C" void foeResourceImportCreateInfo(foeResource resource) {
    auto *pResource = resource_from_handle(resource);

    foeResourceIncrementRefCount(resource);

    bool expected = false;
    if (!pResource->isLoading.compare_exchange_strong(expected, true)) {
        FOE_LOG(foeResourceCore, Warning, "[{},{}] foeResource - Attempted to load in parrallel",
                foeIdToString(pResource->id), pResource->type)
        foeResourceDecrementRefCount(resource);
        return;
    }

    if (pResource->pResourceFns->scheduleAsyncTask != nullptr) {
        FOE_LOG(foeResourceCore, Verbose,
                "[{},{}] foeResource - Importing CreateInfo asynchronously",
                foeIdToString(pResource->id), pResource->type)

        pResource->pResourceFns->scheduleAsyncTask(
            pResource->pResourceFns->pScheduleAsyncTaskContext, (PFN_foeTask)loadCreateInfoTask,
            pResource);
    } else {
        FOE_LOG(foeResourceCore, Verbose,
                "[{},{}] foeResource - Importing CreateInfo synchronously", pResource->id,
                pResource->type)

        loadCreateInfoTask(pResource);
    }
}

namespace {

void postLoadFn(
    foeResource resource,
    foeResultSet loadResult,
    void *pSrc,
    void (*pMoveDataFn)(void *, void *),
    foeResourceCreateInfo createInfo,
    void *pUnloadContext,
    void (*pUnloadFn)(void *, foeResource, uint32_t, PFN_foeResourceUnloadCall *, bool)) {
    auto *pResource = resource_from_handle(resource);
    foeResourceCreateInfo oldCreateInfo{FOE_NULL_HANDLE};

    if (loadResult.value != FOE_SUCCESS) {
        // Loading didn't go well
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        loadResult.toString(loadResult.value, buffer);
        FOE_LOG(foeResourceCore, Error, "[{},{}] foeResource - Failed to load  with error: {}",
                foeIdToString(pResource->id), pResource->type, buffer)

        auto expected = FOE_RESOURCE_LOAD_STATE_UNLOADED;
        pResource->state.compare_exchange_strong(expected, FOE_RESOURCE_LOAD_STATE_FAILED);
        pResource->isLoading = false;
    } else {
        // It loaded successfully and the data is ready to be moved now
        foeResourceCreateInfoIncrementRefCount(createInfo);

        pResource->sync.lock();

        // Unload any previous data
        foeResourceUnload(resource, true);

        // Move the new data in
        pMoveDataFn(pSrc, (void *)foeResourceGetData(resource));

        oldCreateInfo = pResource->loadedCreateInfo;

        pResource->loadedCreateInfo = createInfo;
        pResource->pUnloadContext = pUnloadContext;
        pResource->pUnloadFn = pUnloadFn;

        pResource->state = FOE_RESOURCE_LOAD_STATE_LOADED;
        pResource->sync.unlock();
    }

    pResource->isLoading = false;
    foeResourceDecrementRefCount(resource);

    // If destroying old create info data, do it outside the critical area, could be expensive
    if (oldCreateInfo != FOE_NULL_HANDLE) {
        auto refCount = foeResourceCreateInfoDecrementRefCount(oldCreateInfo);
        if (refCount == 0) {
            foeDestroyResourceCreateInfo(oldCreateInfo);
        }
    }
}

struct LoadTaskData {
    foeResourceImpl *pResource;
    bool refreshCreateInfo;
};

void loadResourceTask(LoadTaskData *pContext) {
    auto const &pResource = pContext->pResource;

    auto createInfo = foeResourceGetCreateInfo(resource_to_handle(pResource));

    if (pContext->refreshCreateInfo || createInfo == FOE_NULL_HANDLE) {
        foeResourceCreateInfo oldCreateInfo{FOE_NULL_HANDLE};

        if (createInfo != FOE_NULL_HANDLE)
            foeResourceCreateInfoDecrementRefCount(createInfo);

        createInfo = pResource->pResourceFns->pImportFn(pResource->pResourceFns->pImportContext,
                                                        pResource->id);

        pResource->sync.lock();
        if (createInfo != FOE_NULL_HANDLE) {
            oldCreateInfo = pResource->createInfo;
        }
        pResource->createInfo = createInfo;
        pResource->sync.unlock();

        // If destroying old create info data, do it outside the locked area, could be expensive
        if (oldCreateInfo != FOE_NULL_HANDLE) {
            auto refCount = foeResourceCreateInfoDecrementRefCount(oldCreateInfo);
            if (refCount == 0) {
                foeDestroyResourceCreateInfo(oldCreateInfo);
            }
        }
    }

    if (pResource->createInfo != FOE_NULL_HANDLE) {
        pResource->pResourceFns->pLoadFn(pResource->pResourceFns->pLoadContext,
                                         resource_to_handle(pResource), postLoadFn);
    } else {
        postLoadFn(resource_to_handle(pResource), to_foeResult(FOE_RESOURCE_ERROR_NO_CREATE_INFO),
                   nullptr, nullptr, nullptr, nullptr, nullptr);
    }

    // Free the heap-allocated context data
    free(pContext);
}

} // namespace

extern "C" void foeResourceLoad(foeResource resource, bool refreshCreateInfo) {
    auto *pResource = resource_from_handle(resource);

    foeResourceIncrementRefCount(resource);

    bool expected = false;
    if (!pResource->isLoading.compare_exchange_strong(expected, true)) {
        FOE_LOG(foeResourceCore, Warning, "[{},{}] foeResource - Attempted to load in parrallel",
                foeIdToString(pResource->id), pResource->type)
        foeResourceDecrementRefCount(resource);
        return;
    }

    LoadTaskData *pTaskContext = (LoadTaskData *)malloc(sizeof(LoadTaskData));
    *pTaskContext = {
        .pResource = pResource,
        .refreshCreateInfo = refreshCreateInfo,
    };

    if (pResource->pResourceFns->scheduleAsyncTask != nullptr) {
        FOE_LOG(foeResourceCore, Verbose,
                "[{},{}] foeResource - Importing CreateInfo asynchronously",
                foeIdToString(pResource->id), pResource->type)

        pResource->pResourceFns->scheduleAsyncTask(
            pResource->pResourceFns->pScheduleAsyncTaskContext, (PFN_foeTask)loadResourceTask,
            pTaskContext);
    } else {
        FOE_LOG(foeResourceCore, Verbose, "[{},{}] foeResource - Loading synchronously",
                foeIdToString(pResource->id), pResource->type)

        loadResourceTask(pTaskContext);
    }
}

namespace {

bool resourceUnloadCall(foeResource resource,
                        uint32_t iteration,
                        void *pDst,
                        void (*pMoveFn)(void *, void *)) {
    auto *pResource = resource_from_handle(resource);
    bool retVal{false};
    foeResourceCreateInfo oldCreateInfo{FOE_NULL_HANDLE};

    pResource->sync.lock();

    if (iteration == pResource->iteration) {
        retVal = true;

        pMoveFn((void *)foeResourceGetData(resource), pDst);

        oldCreateInfo = pResource->loadedCreateInfo;

        pResource->loadedCreateInfo = FOE_NULL_HANDLE;
        pResource->pUnloadContext = nullptr;
        pResource->pUnloadFn = nullptr;
        pResource->state = FOE_RESOURCE_LOAD_STATE_UNLOADED;

        ++pResource->iteration;
    }

    pResource->sync.unlock();

    // To prevent issues due to maybe slower destruction of ResourceCreateInfo,
    // decrement/destroy it outside the sync-locked portion
    if (oldCreateInfo != FOE_NULL_HANDLE) {
        auto refCount = foeResourceCreateInfoDecrementRefCount(oldCreateInfo);
        if (refCount == 0)
            foeDestroyResourceCreateInfo(oldCreateInfo);
    }

    return retVal;
}

} // namespace

extern "C" void foeResourceUnload(foeResource resource, bool immediate) {
    auto *pResource = resource_from_handle(resource);

    pResource->sync.lock();

    if (pResource->pUnloadFn != nullptr) {
        if (int uses = pResource->useCount; uses > 0) {
            FOE_LOG(foeResourceCore, Warning,
                    "[{},{}] foeResource - Unloading while still actively used {} times",
                    foeIdToString(pResource->id), pResource->type, uses)
        }

        if (immediate) {
            FOE_LOG(foeResourceCore, Verbose, "[{},{}] foeResource - Unloading immediately",
                    foeIdToString(pResource->id), pResource->type)
        } else {
            FOE_LOG(foeResourceCore, Verbose, "[{},{}] foeResource - Unloading normally",
                    foeIdToString(pResource->id), pResource->type)
        }

        pResource->pUnloadFn(pResource->pUnloadContext, resource, pResource->iteration,
                             resourceUnloadCall, immediate);
    }

    pResource->sync.unlock();
}

foeResourceCreateInfo foeResourceGetCreateInfo(foeResource resource) {
    auto *pResource = resource_from_handle(resource);

    pResource->sync.lock();

    foeResourceCreateInfo createInfo = pResource->createInfo;
    if (createInfo != FOE_NULL_HANDLE)
        foeResourceCreateInfoIncrementRefCount(createInfo);

    pResource->sync.unlock();

    return createInfo;
}