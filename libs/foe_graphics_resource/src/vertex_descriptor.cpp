/*
    Copyright (C) 2021 George Cave.

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

#include <foe/graphics/resource/vertex_descriptor.hpp>

#include <foe/simulation/core/resource_fns.hpp>

#include "log.hpp"

foeVertexDescriptor::foeVertexDescriptor(foeResourceID resource,
                                         foeResourceFns const *pResourceFns) :
    foeResourceBase{resource, pResourceFns} {}

foeVertexDescriptor::~foeVertexDescriptor() {
    if (useCount > 0) {
        FOE_LOG(foeGraphicsResource, Warning,
                "foeVertexDescriptor {} being destroyed despite having active uses",
                foeIdToString(resource));
    }
    if (refCount > 0) {
        FOE_LOG(foeGraphicsResource, Warning,
                "foeVertexDescriptor {} being destroyed despite having active references",
                foeIdToString(resource));
    }
}

void foeVertexDescriptor::loadCreateInfo() {
    incrementRefCount();

    // Acquire exclusive loading rights
    bool expected{false};
    if (!isLoading.compare_exchange_strong(expected, true)) {
        // Another thread is already loading this resource
        FOE_LOG(foeGraphicsResource, Warning,
                "Attempted to load foeVertexDescriptor {} in parrallel", foeIdToString(resource))
        decrementRefCount();
        return;
    }

    pResourceFns->asyncTaskFn([this]() {
        auto *pNewCreateInfo = pResourceFns->pImportFn(pResourceFns->pImportContext, resource);
        if (pNewCreateInfo != nullptr) {
            pCreateInfo.reset(pNewCreateInfo);
        }

        isLoading = false;
        decrementRefCount();
    });
}

namespace {

void postLoadFn(void *pResource, std::error_code errC) {
    auto *pImage = reinterpret_cast<foeVertexDescriptor *>(pResource);

    if (!errC) {
        // Loading went fine
        pImage->state = foeResourceState::Loaded;
    } else {
        // Loading didn't go well
        FOE_LOG(foeGraphicsResource, Error,
                "Failed to load foeVertexDescriptor {} with error {}:{}",
                foeIdToString(pImage->getID()), errC.value(), errC.message())
        auto expected = foeResourceState::Unloaded;
        pImage->state.compare_exchange_strong(expected, foeResourceState::Failed);
    }

    pImage->isLoading = false;
    pImage->decrementRefCount();
}

} // namespace

void foeVertexDescriptor::loadResource(bool refreshCreateInfo) {
    incrementRefCount();

    // Acquire exclusive loading rights
    bool expected{false};
    if (!isLoading.compare_exchange_strong(expected, true)) {
        // Another thread is already loading this resource
        FOE_LOG(foeGraphicsResource, Warning,
                "Attempted to load foeVertexDescriptor {} in parrallel", foeIdToString(resource))
        decrementRefCount();
        return;
    }

    pResourceFns->asyncTaskFn([this, refreshCreateInfo]() {
        auto pLocalCreateInfo = pCreateInfo;

        if (refreshCreateInfo || pLocalCreateInfo == nullptr) {
            auto *pNewCreateInfo = pResourceFns->pImportFn(pResourceFns->pImportContext, resource);
            if (pNewCreateInfo != nullptr) {
                pLocalCreateInfo.reset(pNewCreateInfo);
            }
            pCreateInfo = pLocalCreateInfo;
        }

        pResourceFns->pLoadFn(pResourceFns->pLoadContext, this, postLoadFn);
    });
}

void foeVertexDescriptor::unloadResource() {
    modifySync.lock();
    if (data.pUnloadFn != nullptr) {
        data.pUnloadFn(data.pUnloadContext, this, iteration, false);
    }
    modifySync.unlock();
}