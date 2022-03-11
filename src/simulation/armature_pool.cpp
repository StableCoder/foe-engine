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

#include <mutex>

foeArmaturePool::foeArmaturePool(foeResourceFns const &resourceFns) : mResourceFns{resourceFns} {}

foeArmaturePool::~foeArmaturePool() {
    for (auto const resource : mResources) {
        foeResourceDecrementRefCount(resource);
        foeDestroyResource(resource);
    }
}

foeResource foeArmaturePool::add(foeResourceID resource) {
    std::scoped_lock lock{mSync};

    // If it finds it, return nullptr
    for (auto const it : mResources) {
        if (foeResourceGetID(it) == resource) {
            return FOE_NULL_HANDLE;
        }
    }

    // Not found, add it
    foeResource newResource;
    std::error_code errC = foeCreateResource(resource, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE,
                                             &mResourceFns, sizeof(foeArmature), &newResource);
    if (errC)
        return FOE_NULL_HANDLE;

    foeResourceIncrementRefCount(newResource);

    mResources.emplace_back(newResource);

    return newResource;
}

foeResource foeArmaturePool::findOrAdd(foeResourceID resource) {
    std::scoped_lock lock{mSync};

    for (auto const it : mResources) {
        if (foeResourceGetID(it) == resource) {
            return it;
        }
    }

    // Not found, create it now
    foeResource newResource;
    std::error_code errC = foeCreateResource(resource, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE,
                                             &mResourceFns, sizeof(foeArmature), &newResource);
    if (errC)
        return FOE_NULL_HANDLE;

    foeResourceIncrementRefCount(newResource);

    mResources.emplace_back(newResource);

    return newResource;
}

foeResource foeArmaturePool::find(foeResourceID resource) {
    foeResource outResource;

    mSync.lock_shared();
    for (auto const it : mResources) {
        if (foeResourceGetID(it) == resource) {
            outResource = it;
            break;
        }
    }
    mSync.unlock_shared();

    return outResource;
}

void foeArmaturePool::setAsyncTaskFn(std::function<void(std::function<void()>)> asyncTaskFn) {
    mResourceFns.asyncTaskFn = asyncTaskFn;
}

void foeArmaturePool::unloadAll() {
    std::scoped_lock lock{mSync};

    for (auto const it : mResources) {
        foeResourceUnload(it, false);
    }
}