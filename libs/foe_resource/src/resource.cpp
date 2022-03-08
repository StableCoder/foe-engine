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
    void (*pDestroyFn)(foeResourceType, void *);

    std::recursive_mutex sync;

    std::atomic_int refCount;
    std::atomic_int useCount;
    std::atomic_uint iteration;

    std::atomic_bool isLoading{false};
    std::atomic<foeResourceState> state{foeResourceState::Unloaded};

    std::shared_ptr<foeResourceCreateInfoBase> pCreateInfo;

    foeResourceImpl(foeResourceID id,
                    foeResourceType type,
                    foeResourceFns *pResourceFns,
                    void (*pDestroyFn)(foeResourceType, void *)) :
        id{id}, type{type}, pResourceFns{pResourceFns}, pDestroyFn{pDestroyFn} {}
};

FOE_DEFINE_HANDLE_CASTS(resource, foeResourceImpl, foeResource)

} // namespace

extern "C" foeErrorCode foeCreateResource(foeResourceID id,
                                          foeResourceType type,
                                          foeResourceFns *pResourceFns,
                                          void (*pDestroyFn)(foeResourceType, void *),
                                          size_t size,
                                          foeResource *pResource) {
    auto *pNewResource = (foeResourceImpl *)malloc(sizeof(foeResourceImpl) + size);
    if (pNewResource == nullptr) {
        return std::error_code{FOE_RESOURCE_ERROR_OUT_OF_HOST_MEMORY};
    }

    new (pNewResource) foeResourceImpl(id, type, pResourceFns, pDestroyFn);

    *pResource = resource_to_handle(pNewResource);

    FOE_LOG(foeResourceCore, Verbose, "foeResource[{},{}] - Created @ {}", id, type,
            (void *)pNewResource)

    return std::error_code{FOE_RESOURCE_SUCCESS};
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

    if (pResource->pDestroyFn != nullptr) {
        pResource->pDestroyFn(pResource->type, foeResourceGetData(resource));
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

extern "C" foeResourceState foeResourceGetState(foeResource resource) {
    auto *pResource = resource_from_handle(resource);
    return pResource->state;
}

extern "C" void *foeResourceGetData(foeResource resource) {
    auto *pResource = resource_from_handle(resource);
    return (char *)pResource + sizeof(foeResourceImpl);
}