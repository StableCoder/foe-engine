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

#include <foe/physics/resource/collision_shape.hpp>
#include <foe/simulation/core/resource_fns.hpp>

#include "log.hpp"

foeCollisionShape::foeCollisionShape(foeResourceID resource, foeResourceFns const *pResourceFns) :
    foeResourceBase{resource, pResourceFns} {}

foeCollisionShape::~foeCollisionShape() {
    if (useCount > 0) {
        FOE_LOG(foePhysics, Warning,
                "foeCollisionShape {} being destroyed despite having active uses",
                foeIdToString(resource));
    }
    if (refCount > 0) {
        FOE_LOG(foePhysics, Warning,
                "foeCollisionShape {} being destroyed despite having active references",
                foeIdToString(resource));
    }
}

void foeCollisionShape::loadCreateInfo() {
    incrementRefCount();

    // Acquire exclusive loading rights
    bool expected{false};
    if (!isLoading.compare_exchange_strong(expected, true)) {
        // Another thread is already loading this resource
        FOE_LOG(foePhysics, Warning, "Attempted to load foeCollisionShape {} in parrallel",
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
        FOE_LOG(foePhysics, Verbose, "Creating foeCollisionShape {} asynchronously",
                foeIdToString(resource))
        pResourceFns->asyncTaskFn(createFn);
    } else {
        FOE_LOG(foePhysics, Verbose, "Creating foeCollisionShape {} synchronously",
                foeIdToString(resource))
        createFn();
    }
}

namespace {

void postLoadFn(void *pResource, std::error_code errC) {
    auto *pCollisionShape = reinterpret_cast<foeCollisionShape *>(pResource);

    if (!errC) {
        // Loading went fine
        pCollisionShape->state = foeResourceState::Loaded;
    } else {
        // Loading didn't go well
        FOE_LOG(foePhysics, Error, "Failed to load foeCollisionShape {} with error {}:{}",
                foeIdToString(pCollisionShape->getID()), errC.value(), errC.message())
        auto expected = foeResourceState::Unloaded;
        pCollisionShape->state.compare_exchange_strong(expected, foeResourceState::Failed);
    }

    pCollisionShape->isLoading = false;
    pCollisionShape->decrementRefCount();
}

} // namespace

void foeCollisionShape::loadResource(bool refreshCreateInfo) {
    incrementRefCount();

    // Acquire exclusive loading rights
    bool expected{false};
    if (!isLoading.compare_exchange_strong(expected, true)) {
        // Another thread is already loading this resource
        FOE_LOG(foePhysics, Warning, "Attempted to load foeCollisionShape {} in parrallel",
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
        FOE_LOG(foePhysics, Verbose, "Loading foeCollisionShape {} asynchronously",
                foeIdToString(resource))
        pResourceFns->asyncTaskFn(loadFn);
    } else {
        FOE_LOG(foePhysics, Verbose, "Loading foeCollisionShape {} synchronously",
                foeIdToString(resource))
        loadFn();
    }
}

void foeCollisionShape::unloadResource() {
    modifySync.lock();
    if (data.pUnloadFn != nullptr) {
        data.pUnloadFn(data.pUnloadContext, this, iteration, false);
    }
    modifySync.unlock();
}