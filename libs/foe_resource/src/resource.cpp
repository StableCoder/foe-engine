// Copyright (C) 2022 George Cave.
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

enum ResourceLoadingFlagBits : uint8_t {
    None = 0,
    CreateInfo = 0x01,
    Data = 0x02,
};
typedef uint8_t ResourceLoadingFlags;

struct foeResourceImpl {
    foeResourceID id;
    foeResourceType type;

    foeResourceFns const *pResourceFns;

    std::recursive_mutex sync;

    std::atomic_int refCount{1};
    std::atomic_int useCount{0};

    // Create Info State
    foeResourceCreateInfo createInfo{FOE_NULL_HANDLE};

    // Load State
    std::atomic_uint iteration{0};
    std::atomic<ResourceLoadingFlags> loading{ResourceLoadingFlagBits::None};
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

    FOE_LOG(foeResource, FOE_LOG_LEVEL_VERBOSE, "[{},{}] foeResource - Created @ {}",
            foeIdToString(id), type, (void *)pNewResource)

    return to_foeResult(FOE_RESOURCE_SUCCESS);
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
    int refCount = --pResource->refCount;

    if (refCount == 0) {
        FOE_LOG(foeResource, FOE_LOG_LEVEL_VERBOSE, "[{},{}] foeResource - Destroying",
                foeIdToString(pResource->id), pResource->type)

        int useCount = pResource->useCount;
        if (useCount != 0) {
            FOE_LOG(foeResource, FOE_LOG_LEVEL_WARNING,
                    "[{},{}] foeResource - Destroying with a non-zero use count of: {}",
                    foeIdToString(pResource->id), pResource->type, useCount)
        }

        foeResourceUnloadData(resource, true);

        // Clear createInfo
        if (pResource->createInfo != FOE_NULL_HANDLE) {
            foeResourceCreateInfoDecrementRefCount(pResource->createInfo);
        }

        FOE_LOG(foeResource, FOE_LOG_LEVEL_VERBOSE, "[{},{}] foeResource - Destroyed",
                foeIdToString(pResource->id), pResource->type)

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
    return pResource->loading != ResourceLoadingFlagBits::None;
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

    pResource->sync.lock();
    if (newCreateInfo != FOE_NULL_HANDLE) {
        oldCreateInfo = pResource->createInfo;
    }
    pResource->createInfo = newCreateInfo;
    pResource->sync.unlock();

    // Remove the loading/CI flag
    ResourceLoadingFlags expected = pResource->loading;
    ResourceLoadingFlags desired;
    do {
        desired = expected ^ ResourceLoadingFlagBits::CreateInfo;
    } while (!pResource->loading.compare_exchange_weak(expected, desired));

    foeResourceDecrementRefCount(resource_to_handle(pResource));

    // If destroying old create info data, do it outside the locked area, could be expensive
    if (oldCreateInfo != FOE_NULL_HANDLE) {
        foeResourceCreateInfoDecrementRefCount(oldCreateInfo);
    }
}

} // namespace

extern "C" void foeResourceLoadCreateInfo(foeResource resource) {
    auto *pResource = resource_from_handle(resource);

    foeResourceIncrementRefCount(resource);

    ResourceLoadingFlags expected = ResourceLoadingFlagBits::None;
    if (!pResource->loading.compare_exchange_strong(expected,
                                                    ResourceLoadingFlagBits::CreateInfo)) {
        FOE_LOG(foeResource, FOE_LOG_LEVEL_WARNING,
                "[{},{}] foeResource - Attempted to load CreateInfo in parrallel",
                foeIdToString(pResource->id), pResource->type)
        foeResourceDecrementRefCount(resource);
        return;
    }

    if (pResource->pResourceFns->scheduleAsyncTask != nullptr) {
        FOE_LOG(foeResource, FOE_LOG_LEVEL_VERBOSE,
                "[{},{}] foeResource - Loading CreateInfo asynchronously",
                foeIdToString(pResource->id), pResource->type)

        pResource->pResourceFns->scheduleAsyncTask(
            pResource->pResourceFns->pScheduleAsyncTaskContext, (PFN_foeTask)loadCreateInfoTask,
            pResource);
    } else {
        FOE_LOG(foeResource, FOE_LOG_LEVEL_VERBOSE,
                "[{},{}] foeResource - Loading CreateInfo synchronously", pResource->id,
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

        // Since we're not going to be using it, decrement which may destroy it
        if (createInfo != FOE_NULL_HANDLE)
            foeResourceCreateInfoDecrementRefCount(createInfo);

        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        loadResult.toString(loadResult.value, buffer);
        FOE_LOG(foeResource, FOE_LOG_LEVEL_ERROR,
                "[{},{}] foeResource - Failed to load  with error: {}",
                foeIdToString(pResource->id), pResource->type, buffer)

        auto expected = FOE_RESOURCE_LOAD_STATE_UNLOADED;
        pResource->state.compare_exchange_strong(expected, FOE_RESOURCE_LOAD_STATE_FAILED);
        pResource->loading = ResourceLoadingFlagBits::None;
    } else {
        pResource->sync.lock();

        // Unload any previous data
        foeResourceUnloadData(resource, true);

        // Move the new data in
        pMoveDataFn(pSrc, (void *)foeResourceGetData(resource));

        oldCreateInfo = pResource->loadedCreateInfo;

        pResource->loadedCreateInfo = createInfo;
        pResource->pUnloadContext = pUnloadContext;
        pResource->pUnloadFn = pUnloadFn;

        pResource->state = FOE_RESOURCE_LOAD_STATE_LOADED;
        pResource->sync.unlock();
    }

    pResource->loading = ResourceLoadingFlagBits::None;
    foeResourceDecrementRefCount(resource);

    // If destroying old create info data, do it outside the critical area, could be expensive
    if (oldCreateInfo != FOE_NULL_HANDLE) {
        foeResourceCreateInfoDecrementRefCount(oldCreateInfo);
    }
}

struct LoadTaskData {
    foeResourceImpl *pResource;
    ResourceLoadingFlags loadingFlags;
};

void loadResourceTask(LoadTaskData *pContext) {
    foeResourceImpl *pResource = pContext->pResource;

    foeResourceCreateInfo createInfo = foeResourceGetCreateInfo(resource_to_handle(pResource));

    if (createInfo == FOE_NULL_HANDLE) {
        if (pContext->loadingFlags & ResourceLoadingFlagBits::CreateInfo) {
            // We're responsible for loading flag bits
            foeResourceIncrementRefCount(resource_to_handle(pResource));
            loadCreateInfoTask(pResource);
        } else {
            // Another thread is loading it, keep yielding until it's done
            // @todo Possibly look into re-queuing via async call instead?
            while (pResource->loading.load() & ResourceLoadingFlagBits::CreateInfo) {
                std::this_thread::yield();
            }
        }
    } else {
        foeResourceCreateInfoDecrementRefCount(createInfo);
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

extern "C" void foeResourceLoadData(foeResource resource) {
    auto *pResource = resource_from_handle(resource);

    foeResourceIncrementRefCount(resource);

    ResourceLoadingFlags expected = pResource->loading;
    // Only want to start loading if the data isn't already slated to be loaded
    if (expected & ResourceLoadingFlagBits::Data ||
        !pResource->loading.compare_exchange_strong(expected,
                                                    expected | ResourceLoadingFlagBits::CreateInfo |
                                                        ResourceLoadingFlagBits::Data)) {
        FOE_LOG(foeResource, FOE_LOG_LEVEL_WARNING,
                "[{},{}] foeResource - Attempted to load in parrallel",
                foeIdToString(pResource->id), pResource->type)
        foeResourceDecrementRefCount(resource);
        return;
    }

    LoadTaskData *pTaskContext = (LoadTaskData *)malloc(sizeof(LoadTaskData));
    expected ^= ResourceLoadingFlagBits::CreateInfo | ResourceLoadingFlagBits::Data;
    *pTaskContext = {
        .pResource = pResource,
        .loadingFlags = expected,
    };

    if (pResource->pResourceFns->scheduleAsyncTask != nullptr) {
        FOE_LOG(foeResource, FOE_LOG_LEVEL_VERBOSE,
                "[{},{}] foeResource - Importing CreateInfo asynchronously",
                foeIdToString(pResource->id), pResource->type)

        pResource->pResourceFns->scheduleAsyncTask(
            pResource->pResourceFns->pScheduleAsyncTaskContext, (PFN_foeTask)loadResourceTask,
            pTaskContext);
    } else {
        FOE_LOG(foeResource, FOE_LOG_LEVEL_VERBOSE, "[{},{}] foeResource - Loading synchronously",
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
        foeResourceCreateInfoDecrementRefCount(oldCreateInfo);
    }

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
                    foeIdToString(pResource->id), pResource->type, uses)
        }

        if (immediate) {
            FOE_LOG(foeResource, FOE_LOG_LEVEL_VERBOSE,
                    "[{},{}] foeResource - Unloading immediately", foeIdToString(pResource->id),
                    pResource->type)
        } else {
            FOE_LOG(foeResource, FOE_LOG_LEVEL_VERBOSE, "[{},{}] foeResource - Unloading normally",
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