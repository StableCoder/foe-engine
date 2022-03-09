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

#include <foe/simulation/core/create_info.hpp>
#include <foe/simulation/core/resource_fns.hpp>

#include <atomic>
#include <memory>
#include <mutex>

#include "error_code.hpp"
#include "log.hpp"

struct foeResourceFns;

namespace {

struct foeResourceImpl {
    foeResourceID id;
    foeResourceType type;

    foeResourceFns *pResourceFns;

    std::recursive_mutex sync;

    std::atomic_int refCount;
    std::atomic_int useCount;

    // Create Info State
    std::shared_ptr<foeResourceCreateInfoBase> pCreateInfo;

    // Load State
    std::atomic_uint iteration;
    std::atomic_bool isLoading{false};
    std::atomic<foeResourceLoadState> state{foeResourceLoadState::Unloaded};

    std::shared_ptr<foeResourceCreateInfoBase> pLoadedCreateInfo;
    void *pUnloadContext{nullptr};
    void (*pUnloadFn)(void *, foeResource, uint32_t, PFN_foeResourceUnloadCall *, bool){nullptr};

    foeResourceImpl(foeResourceID id, foeResourceType type, foeResourceFns *pResourceFns) :
        id{id}, type{type}, pResourceFns{pResourceFns} {}
};

FOE_DEFINE_HANDLE_CASTS(resource, foeResourceImpl, foeResource)

void *foeResourceGetMutableData(foeResource resource) {
    auto *pResource = resource_from_handle(resource);
    return (char *)pResource + sizeof(foeResourceImpl);
}

} // namespace

extern "C" foeErrorCode foeCreateResource(foeResourceID id,
                                          foeResourceType type,
                                          foeResourceFns *pResourceFns,
                                          size_t size,
                                          foeResource *pResource) {
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
        auto *pNewCreateInfo = pResource->pResourceFns->pImportFn(
            pResource->pResourceFns->pImportContext, pResource->id);
        if (pNewCreateInfo != nullptr) {
            pResource->pCreateInfo.reset(pNewCreateInfo);
        }

        pResource->isLoading = false;
        foeResourceDecrementRefCount(resource);
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
    std::shared_ptr<foeResourceCreateInfoBase> pCreateInfo,
    void *pUnloadContext,
    void (*pUnloadFn)(void *, foeResource, uint32_t, PFN_foeResourceUnloadCall *, bool)) {
    auto *pResource = resource_from_handle(resource);
    std::error_code errC = errorCode;

    if (errC) {
        // Loading didn't go well
        FOE_LOG(foeResourceCore, Error, "foeResource[{},{}] - Failed to load  with error {}",
                pResource->id, pResource->type, errC.message())
        auto expected = foeResourceLoadState::Unloaded;
        pResource->state.compare_exchange_strong(expected, foeResourceLoadState::Failed);
    } else {
        // It loaded successfully and the data is ready to be moved now
        pResource->sync.lock();

        // Unload any previous data
        foeResourceUnload(resource, true);

        // Move the new data in
        pMoveDataFn(pSrc, foeResourceGetMutableData(resource));

        pResource->pLoadedCreateInfo = std::move(pCreateInfo);
        pResource->pUnloadContext = pUnloadContext;
        pResource->pUnloadFn = pUnloadFn;

        pResource->state = foeResourceLoadState::Loaded;

        pResource->sync.unlock();
    }

    pResource->isLoading = false;
    foeResourceDecrementRefCount(resource);
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

    auto loadFn = [pResource, refreshCreateInfo]() {
        auto pLocalCreateInfo = pResource->pCreateInfo;

        if (refreshCreateInfo || pLocalCreateInfo == nullptr) {
            auto *pNewCreateInfo = pResource->pResourceFns->pImportFn(
                pResource->pResourceFns->pImportContext, pResource->id);
            if (pNewCreateInfo != nullptr) {
                pLocalCreateInfo.reset(pNewCreateInfo);
            }
            pResource->pCreateInfo = pLocalCreateInfo;
        }

        pResource->pResourceFns->pLoadFn2(pResource->pResourceFns->pLoadContext,
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
    bool retVal{false};
    auto *pResource = resource_from_handle(resource);

    pResource->sync.lock();

    if (iteration == pResource->iteration) {
        retVal = true;

        pMoveFn(foeResourceGetMutableData(resource), pDst);

        pResource->pLoadedCreateInfo.reset();
        pResource->pUnloadContext = nullptr;
        pResource->pUnloadFn = nullptr;

        ++pResource->iteration;
    }

    pResource->sync.unlock();

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

std::shared_ptr<foeResourceCreateInfoBase> foeResourceGetCreateInfo(foeResource resource) {
    auto *pResource = resource_from_handle(resource);
    return pResource->pCreateInfo;
}