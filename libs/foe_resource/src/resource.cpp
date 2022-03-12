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

#include <foe/resource/resource.h>

#include <foe/resource/resource_fns.hpp>

#include <atomic>
#include <mutex>

#include "error_code.hpp"
#include "log.hpp"

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
    std::atomic<foeResourceLoadState> state{foeResourceLoadState::Unloaded};

    foeResourceCreateInfo loadedCreateInfo{FOE_NULL_HANDLE};
    void *pUnloadContext{nullptr};
    void (*pUnloadFn)(void *, foeResource, uint32_t, PFN_foeResourceUnloadCall *, bool){nullptr};

    foeResourceImpl(foeResourceID id, foeResourceType type, foeResourceFns const *pResourceFns) :
        id{id}, type{type}, pResourceFns{pResourceFns} {}
};

FOE_DEFINE_HANDLE_CASTS(resource, foeResourceImpl, foeResource)

} // namespace

extern "C" foeErrorCode foeCreateResource(foeResourceID id,
                                          foeResourceType type,
                                          foeResourceFns const *pResourceFns,
                                          size_t size,
                                          foeResource *pResource) {
    if (pResourceFns == nullptr)
        return foeToErrorCode(FOE_RESOURCE_ERROR_RESOURCE_FUNCTIONS_NOT_PROVIDED);

    auto *pNewResource = (foeResourceImpl *)malloc(sizeof(foeResourceImpl) + size);
    if (pNewResource == nullptr) {
        return foeToErrorCode(FOE_RESOURCE_ERROR_OUT_OF_HOST_MEMORY);
    }

    new (pNewResource) foeResourceImpl(id, type, pResourceFns);

    *pResource = resource_to_handle(pNewResource);

    FOE_LOG(foeResourceCore, Verbose, "foeResource[{},{}] - Created @ {}", id, type,
            (void *)pNewResource)

    return foeToErrorCode(FOE_RESOURCE_SUCCESS);
}

extern "C" void foeDestroyResource(foeResource resource) {
    auto *pResource = resource_from_handle(resource);

    if (pResource->useCount > 0) {
        FOE_LOG(foeResourceCore, Warning,
                "foeResource[{},{}] - Destroying while use count is non-zero", pResource->id,
                pResource->type)
    }
    if (pResource->refCount > 0) {
        FOE_LOG(foeResourceCore, Warning,
                "foeResource[{},{}] - Destroying while reference count is non-zero", pResource->id,
                pResource->type)
    }

    foeResourceUnload(resource, true);

    // Clear createInfo
    if (pResource->createInfo != FOE_NULL_HANDLE) {
        auto count = foeResourceCreateInfoDecrementRefCount(pResource->createInfo);
        if (count == 0) {
            foeDestroyResourceCreateInfo(pResource->createInfo);
        }
    }

    FOE_LOG(foeResourceCore, Verbose, "foeResource[{},{}] - Destroyed", pResource->id,
            pResource->type)

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

extern "C" void foeResourceImportCreateInfo(foeResource resource) {
    auto *pResource = resource_from_handle(resource);

    foeResourceIncrementRefCount(resource);

    bool expected = false;
    if (!pResource->isLoading.compare_exchange_strong(expected, true)) {
        FOE_LOG(foeResourceCore, Warning, "foeResource[{},{}] - Attempted to load in parrallel",
                pResource->id, pResource->type)
        foeResourceDecrementRefCount(resource);
        return;
    }

    auto createFn = [resource, pResource]() {
        foeResourceCreateInfo newCreateInfo = pResource->pResourceFns->pImportFn(
            pResource->pResourceFns->pImportContext, pResource->id);
        foeResourceCreateInfo oldCreateInfo{FOE_NULL_HANDLE};

        foeResourceCreateInfoIncrementRefCount(newCreateInfo);

        pResource->sync.lock();
        if (newCreateInfo != FOE_NULL_HANDLE) {
            oldCreateInfo = pResource->createInfo;

            pResource->createInfo = newCreateInfo;
        }
        pResource->isLoading = false;
        pResource->sync.unlock();

        foeResourceDecrementRefCount(resource);

        // If destroying old create info data, do it outside the locked area, could be expensive
        if (oldCreateInfo != FOE_NULL_HANDLE) {
            auto refCount = foeResourceCreateInfoDecrementRefCount(oldCreateInfo);
            if (refCount == 0) {
                foeDestroyResourceCreateInfo(oldCreateInfo);
            }
        }
    };

    if (pResource->pResourceFns->asyncTaskFn) {
        FOE_LOG(foeResourceCore, Verbose,
                "foeResource[{},{}] - Importing CreateInfo asynchronously", pResource->id,
                pResource->type)
        pResource->pResourceFns->asyncTaskFn(createFn);
    } else {
        FOE_LOG(foeResourceCore, Verbose, "foeResource[{},{}] - Importing CreateInfo synchronously",
                pResource->id, pResource->type)
        createFn();
    }
}

namespace {

void postLoadFn(
    foeResource resource,
    foeErrorCode errorCode,
    void *pSrc,
    void (*pMoveDataFn)(void *, void *),
    foeResourceCreateInfo createInfo,
    void *pUnloadContext,
    void (*pUnloadFn)(void *, foeResource, uint32_t, PFN_foeResourceUnloadCall *, bool)) {
    auto *pResource = resource_from_handle(resource);
    std::error_code errC = errorCode;
    foeResourceCreateInfo oldCreateInfo{FOE_NULL_HANDLE};

    if (errC) {
        // Loading didn't go well
        FOE_LOG(foeResourceCore, Error, "foeResource[{},{}] - Failed to load  with error {}",
                pResource->id, pResource->type, errC.message())
        auto expected = foeResourceLoadState::Unloaded;
        pResource->state.compare_exchange_strong(expected, foeResourceLoadState::Failed);
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

        pResource->state = foeResourceLoadState::Loaded;
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

} // namespace

extern "C" void foeResourceLoad(foeResource resource, bool refreshCreateInfo) {
    auto *pResource = resource_from_handle(resource);

    foeResourceIncrementRefCount(resource);

    bool expected = false;
    if (!pResource->isLoading.compare_exchange_strong(expected, true)) {
        FOE_LOG(foeResourceCore, Warning, "foeResource[{},{}] - Attempted to load in parrallel",
                pResource->id, pResource->type)
        foeResourceDecrementRefCount(resource);
        return;
    }

    auto loadFn = [resource, pResource, refreshCreateInfo]() {
        auto createInfo = foeResourceGetCreateInfo(resource);

        if (refreshCreateInfo || createInfo == FOE_NULL_HANDLE) {
            foeResourceCreateInfo oldCreateInfo{FOE_NULL_HANDLE};

            if (createInfo != FOE_NULL_HANDLE)
                foeResourceCreateInfoDecrementRefCount(createInfo);

            createInfo = pResource->pResourceFns->pImportFn(pResource->pResourceFns->pImportContext,
                                                            pResource->id);

            pResource->sync.lock();
            if (createInfo != FOE_NULL_HANDLE) {
                oldCreateInfo = pResource->createInfo;

                pResource->createInfo = createInfo;
            }
            pResource->sync.unlock();

            // If destroying old create info data, do it outside the locked area, could be expensive
            if (oldCreateInfo != FOE_NULL_HANDLE) {
                auto refCount = foeResourceCreateInfoDecrementRefCount(oldCreateInfo);
                if (refCount == 0) {
                    foeDestroyResourceCreateInfo(oldCreateInfo);
                }
            }
        }

        pResource->pResourceFns->pLoadFn(pResource->pResourceFns->pLoadContext,
                                         resource_to_handle(pResource), postLoadFn);
    };

    if (pResource->pResourceFns->asyncTaskFn) {
        FOE_LOG(foeResourceCore, Verbose, "foeResource[{},{}] - Loading asynchronously",
                pResource->id, pResource->type)
        pResource->pResourceFns->asyncTaskFn(loadFn);
    } else {
        FOE_LOG(foeResourceCore, Verbose, "foeResource[{},{}] - Loading synchronously",
                pResource->id, pResource->type)
        loadFn();
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

        ++pResource->iteration;
    }

    pResource->sync.unlock();

    // To prevent issues due to maybe slower destruction of ResourceCreateInfo, decrement/destroy it
    // outside the sync-locked portion
    if (oldCreateInfo != FOE_NULL_HANDLE) {
        foeResourceCreateInfoDecrementRefCount(oldCreateInfo);
        foeDestroyResourceCreateInfo(oldCreateInfo);
    }

    return retVal;
}

} // namespace

extern "C" void foeResourceUnload(foeResource resource, bool immediate) {
    auto *pResource = resource_from_handle(resource);

    pResource->sync.lock();

    if (pResource->pUnloadFn != nullptr) {
        if (immediate) {
            FOE_LOG(foeResourceCore, Verbose, "foeResource[{},{}] - Unloading immediately",
                    pResource->id, pResource->type)
        } else {
            FOE_LOG(foeResourceCore, Verbose, "foeResource[{},{}] - Unloading normally",
                    pResource->id, pResource->type)
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