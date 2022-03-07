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

#include "armature_pool.hpp"

#include "armature.hpp"
#include "type_defs.h"

foeArmaturePool::foeArmaturePool(foeResourceFns const &resourceFns) : mResourceFns{resourceFns} {}

foeArmaturePool::~foeArmaturePool() {
    for (auto *pResource : mResources) {
        pResource->decrementRefCount();

        delete pResource;
    }
}

foeArmature *foeArmaturePool::add(foeResourceID resource) {
    std::scoped_lock lock{mSync};

    // If it finds it, return nullptr
    for (auto *pResource : mResources) {
        if (pResource->getID() == resource) {
            return nullptr;
        }
    }

    // Not found, add it
    foeArmature *pResource = new foeArmature{resource, &mResourceFns};
    pResource->incrementRefCount();

    mResources.emplace_back(pResource);

    return pResource;
}

foeArmature *foeArmaturePool::findOrAdd(foeResourceID resource) {
    std::scoped_lock lock{mSync};

    for (auto *pResource : mResources) {
        if (pResource->getID() == resource) {
            return pResource;
        }
    }

    // Not found, create it now
    foeArmature *pResource = new foeArmature{resource, &mResourceFns};
    pResource->incrementRefCount();

    mResources.emplace_back(pResource);

    return pResource;
}

foeArmature *foeArmaturePool::find(foeId id) {
    foeArmature *pResource{nullptr};

    mSync.lock_shared();
    for (auto *pOld : mResources) {
        if (pOld->getID() == id) {
            pResource = pOld;
            break;
        }
    }
    mSync.unlock_shared();

    return pResource;
}

void foeArmaturePool::setAsyncTaskFn(std::function<void(std::function<void()>)> asyncTaskFn) {
    mResourceFns.asyncTaskFn = asyncTaskFn;
}

void foeArmaturePool::unloadAll() {
    std::scoped_lock lock{mSync};

    for (auto *pResource : mResources) {
        pResource->unloadResource();
    }
}