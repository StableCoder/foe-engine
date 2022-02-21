/*
    Copyright (C) 2021-2022 George Cave.

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

#include <foe/graphics/resource/material.hpp>

#include <foe/simulation/core/resource_fns.hpp>

#include "log.hpp"

foeMaterial::foeMaterial(foeResourceID resource, foeResourceFns const *pResourceFns) :
    foeResourceBase{resource, pResourceFns} {}

foeMaterial::~foeMaterial() {
    if (useCount > 0) {
        FOE_LOG(foeGraphicsResource, Warning,
                "foeMaterial {} being destroyed despite having active uses",
                foeIdToString(resource));
    }
    if (refCount > 0) {
        FOE_LOG(foeGraphicsResource, Warning,
                "foeMaterial {} being destroyed despite having active references",
                foeIdToString(resource));
    }
}

void foeMaterial::loadCreateInfo() {
    incrementRefCount();

    // Acquire exclusive loading rights
    bool expected{false};
    if (!isLoading.compare_exchange_strong(expected, true)) {
        // Another thread is already loading this resource
        FOE_LOG(foeGraphicsResource, Warning, "Attempted to load foeMaterial {} in parrallel",
                foeIdToString(resource))
        decrementRefCount();
        return;
    }

    auto createFn = [this]() {
        auto *pNewCreateInfo = pResourceFns->pImportFn(pResourceFns->pImportContext, resource);
        if (pNewCreateInfo != nullptr) {
            pCreateInfo.reset(pNewCreateInfo);
        }

        isLoading = false;
        decrementRefCount();
    };

    if (pResourceFns->asyncTaskFn) {
        FOE_LOG(foeGraphicsResource, Verbose, "Creating foeMaterial {} asynchronously",
                foeIdToString(resource))
        pResourceFns->asyncTaskFn(createFn);
    } else {
        FOE_LOG(foeGraphicsResource, Verbose, "Creating foeMaterial {} synchronously",
                foeIdToString(resource))
        createFn();
    }
}

namespace {

void postLoadFn(void *pResource, std::error_code errC) {
    auto *pImage = reinterpret_cast<foeMaterial *>(pResource);

    if (!errC) {
        // Loading went fine
        pImage->state = foeResourceState::Loaded;
    } else {
        // Loading didn't go well
        FOE_LOG(foeGraphicsResource, Error, "Failed to load foeMaterial {} with error {}:{}",
                foeIdToString(pImage->getID()), errC.value(), errC.message())
        auto expected = foeResourceState::Unloaded;
        pImage->state.compare_exchange_strong(expected, foeResourceState::Failed);
    }

    pImage->isLoading = false;
    pImage->decrementRefCount();
}

} // namespace

void foeMaterial::loadResource(bool refreshCreateInfo) {
    incrementRefCount();

    // Acquire exclusive loading rights
    bool expected{false};
    if (!isLoading.compare_exchange_strong(expected, true)) {
        // Another thread is already loading this resource
        FOE_LOG(foeGraphicsResource, Warning, "Attempted to load foeMaterial {} in parrallel",
                foeIdToString(resource))
        decrementRefCount();
        return;
    }

    auto loadFn = [this, refreshCreateInfo]() {
        auto pLocalCreateInfo = pCreateInfo;

        if (refreshCreateInfo || pLocalCreateInfo == nullptr) {
            auto *pNewCreateInfo = pResourceFns->pImportFn(pResourceFns->pImportContext, resource);
            if (pNewCreateInfo != nullptr) {
                pLocalCreateInfo.reset(pNewCreateInfo);
            }
            pCreateInfo = pLocalCreateInfo;
        }

        pResourceFns->pLoadFn(pResourceFns->pLoadContext, this, postLoadFn);
    };

    if (pResourceFns->asyncTaskFn) {
        FOE_LOG(foeGraphicsResource, Verbose, "Loading foeMaterial {} asynchronously",
                foeIdToString(resource))
        pResourceFns->asyncTaskFn(loadFn);
    } else {
        FOE_LOG(foeGraphicsResource, Verbose, "Loading foeMaterial {} synchronously",
                foeIdToString(resource))
        loadFn();
    }
}

void foeMaterial::unloadResource() {
    modifySync.lock();
    if (data.pUnloadFn != nullptr) {
        data.pUnloadFn(data.pUnloadContext, this, iteration, false);
    }
    modifySync.unlock();
}